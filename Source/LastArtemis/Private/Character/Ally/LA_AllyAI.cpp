// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/Ally/LA_AllyAI.h"

#include "InterchangeResult.h"
#include "SNegativeActionButton.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/LA_HUD.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ALA_AllyAI::ALA_AllyAI()
{

}

void ALA_AllyAI::BeginPlay()
{
    Super::BeginPlay();
    // 현재 월드에서 생성되어 있는 ULA_HUD 타입의 모든 위젯을 검색
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULA_HUD::StaticClass());

    // 위젯이 존재한다면
    if (FoundWidgets.Num() > 0)
    {
        if (ULA_HUD* HUD = Cast<ULA_HUD>(FoundWidgets[0]))
        {
            HUD->RegisterAllyAuto(this);
        }
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

float ALA_AllyAI::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("Ally Damage Taken! Health: %f / %f"), CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
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

    Super::Die();

    // 두뇌 정지
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

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 사망 연출
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





