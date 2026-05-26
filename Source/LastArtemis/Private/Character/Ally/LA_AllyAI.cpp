// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/LA_HUD.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ALA_AllyAI::ALA_AllyAI()
{
    // 💡 [팁] 아군 AI의 스탯(기본 체력 등)을 다르게 세팅하고 싶다면,
    // 잡몹들처럼 PostInitializeComponents()를 구현해서 수정해 주는 것이 가장 좋습니다!
}

void ALA_AllyAI::BeginPlay()
{
    Super::BeginPlay();

    // 현재 월드에서 생성되어 있는 ULA_HUD 타입의 모든 위젯을 검색
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULA_HUD::StaticClass());

    // 위젯이 존재한다면 HUD에 아군 자동 등록
    if (FoundWidgets.Num() > 0)
    {
        if (ULA_HUD* HUD = Cast<ULA_HUD>(FoundWidgets[0]))
        {
            HUD->RegisterAllyAuto(this);
        }
    }
}

void ALA_AllyAI::PlayGunFireEffect()
{
    if (!GunFireEffect) return;

    if (UStaticMeshComponent* WeaponMesh = FindComponentByClass<UStaticMeshComponent>())
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            GunFireEffect,
            WeaponMesh,
            FName("Muzzle"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

void ALA_AllyAI::SetTargetEnemy(AActor* TargetEnemy)
{
    ALA_AllyAIController* AllyAIController = Cast<ALA_AllyAIController>(GetController());
    if (!AllyAIController) return;

    UBlackboardComponent* Blackboard = AllyAIController->GetBlackboardComponent();
    if (!Blackboard) return;

    Blackboard->SetValueAsObject(FName("TargetActor"), TargetEnemy);
    Blackboard->SetValueAsBool(FName("IsCommandedTarget"), true);
    UE_LOG(LogTemp, Warning, TEXT("Target Set"));
}

// 🎯 [대수술 부위] 대미지 이중 차감 버그 박멸 및 헬스 컴포넌트 완벽 연동
float ALA_AllyAI::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    // 🎯 1. 부모 클래스의 TakeDamage 파이프라인을 정석 호출합니다.
    // 이 순간 현묵님이 만든 컴포넌트가 작동하여 실드/피 정산과 피격 사운드(HitSound) 출력을 한 큐에 끝냅니다.
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.0f) return 0.0f;

    // 💥 [버그 제거] 직접 CurrentHealth -= ActualDamage; 하던 구형 코드를 삭제했습니다.
    // 이제 사망 판단은 부모의 TakeDamage 스케줄러(혹은 컴포넌트 피 체크)에 의해 자동으로 감지됩니다.

    // 🎯 2. 부모 컴포넌트의 실시간 남은 피를 가져와 피격 연출 흐름 제어 (심볼 에러 영구 박멸)
    if (HealthComponent && HealthComponent->GetCurrentHealth() <= 0.0f)
    {
        // 💡 피가 0 이하가 되면 부모 단에서 자동으로 Die()를 실행시키므로 자식 단에서는 리턴해 줍니다.
        return ActualDamage;
    }
    else
    {
        // 아군 AI 기존 몽타주 인터럽트 및 피격 상태 제어 로직 유지
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
        {
            AnimInstance->Montage_Stop(0.2f);
        }

        if (HitMontage)
        {
            bIsHitAnimationPlaying = true;

            if (GetCharacterMovement())
            {
                GetCharacterMovement()->StopMovementImmediately();
            }

            ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(GetController());
            if (AIController)
            {
                AIController->StopMovement();
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    // 블랙보드에 피격 상태 전달 (BT 노드 제어용)
                    Blackboard->SetValueAsBool(FName("IsHit"), true);
                }
            }

            float MontageLength = PlayAnimMontage(HitMontage);

            FTimerHandle MontageTimerHandle;
            GetWorldTimerManager().SetTimer(MontageTimerHandle, this, &ALA_AllyAI::ResetHitState, MontageLength, false);
        }
    }
    return ActualDamage;
}

void ALA_AllyAI::Die()
{
    if (bIsDead) return;

    // 🎯 3. 부모의 Die()를 가장 먼저 호출하여 공통 사망 처리(bIsDead = true 및 콜리전 해제) 선 수행
    Super::Die();

    // 두뇌 정지 (아군 고유 AI 제어권 완전히 셧다운)
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(GetController());
    if (AIController && AIController->GetBrainComponent())
    {
        AIController->GetBrainComponent()->StopLogic("Ally is Dead");
    }

    // 이동 컴포넌트 정지
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
        GetCharacterMovement()->StopMovementImmediately();
    }

    // 사망 연출 (기존 로직 보존)
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.AddDynamic(this, &ALA_AllyAI::OnDeathMontageEnded);
        }
    }
    else
    {
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
        GetMesh()->SetSimulatePhysics(true);
    }

    SetLifeSpan(5.0f);
}

void ALA_AllyAI::ResetHitState()
{
    bIsHitAnimationPlaying = false;
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        AIController->GetBlackboardComponent()->SetValueAsBool(FName("IsHit"), false);
    }
}

void ALA_AllyAI::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == DeathMontage && !bInterrupted)
    {
        if (USkeletalMeshComponent* SkeletalMeshComp = GetMesh())
        {
            SkeletalMeshComp->bPauseAnims = true;
            SkeletalMeshComp->SetComponentTickEnabled(false);
        }
    }
}

void ALA_AllyAI::ResetFiringState()
{
    bIsFiring = false;
}
