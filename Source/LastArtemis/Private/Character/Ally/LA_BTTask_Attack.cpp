// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_Attack.h"
#include "Character/LA_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"                 // 나이아가라 시스템 이펙트


ULA_BTTask_Attack::ULA_BTTask_Attack()
{
    bNotifyTick = true;
    NodeName = TEXT("Attack");

    AttackRange = 500.f;
    AttackInterval = 1.f;
    Damage = 10.f;
    TotalAmmo = 30;
}

EBTNodeResult::Type ULA_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogTemp, Warning, TEXT("Attack Task Executing!"));

    FAttackMemory* Memory = reinterpret_cast<FAttackMemory*>(NodeMemory);
    if (Memory)
    {
        // 시작 후 바로 공격하도록 쿨타임 충전
        Memory->ElapsedTime = AttackInterval;
    }

    return EBTNodeResult::InProgress;

}

void ULA_BTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FAttackMemory* Memory = reinterpret_cast<FAttackMemory*>(NodeMemory);
    if (!Memory) return;

    Memory->ElapsedTime += DeltaSeconds;

    // Owner 및 World 유효성 검사
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!OwnerPawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    UObject* RawTargetObject = BlackboardComp->GetValueAsObject(FName("TargetActor"));
    ALA_BaseCharacter* Target = Cast<ALA_BaseCharacter>(RawTargetObject);

    if (!Target)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 중단 원인 1: 블랙보드의 Target 값이 강제로 지워졌습니다 (NULL)!"));
        BlackboardComp->ClearValue(FName("TargetActor"));
        BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    else if (!IsValid(Target))
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 중단 원인 2: 적 액터가 엔진에서 강제 삭제(Destroy) 대기 중입니다!"));
        BlackboardComp->ClearValue(FName("TargetActor"));
        BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    else if (Target->bIsDead)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 중단 원인 3: 적의 bIsDead 변수가 켜졌습니다! (체력이 남았는데 켜졌다면 데미지 로직 버그)"));
        BlackboardComp->ClearValue(FName("TargetActor"));
        BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    else if (Target->bIsDead)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨중단 원인 4: 적의 체력이 다 되어 bIsDead가 true가 되었습니다! (적 캐릭터의 죽음 애니메이션/삭제 확인 필요!)"));
        BlackboardComp->ClearValue(FName("TargetActor"));
        BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

#pragma region reload
    // ---------------------------------------------------------
    // 1. [재장전 로직] 재장전 중이라면 총을 쏘지 않고 타이머만 올립니다.
    // ---------------------------------------------------------

    if (Memory->bIsReloading)
    {
        Memory->ReloadTimer += DeltaSeconds;
        if (Memory->ReloadTimer >= 2.0f)
        {
            Memory->CurrentAmmo = TotalAmmo;
            Memory->bIsReloading = false;
            Memory ->ReloadTimer = 0.f;
            UE_LOG(LogTemp, Warning, TEXT("Reload Complete!"));
        }
        return;
    }

#pragma endregion


    // ---------------------------------------------------------
    // 2. 적의 가슴팍 위치 추적 및 회전
    // ---------------------------------------------------------

    // 타겟의 중앙(가슴팍) 좌표 구하기
    FVector TargetChestLocation = Target->GetActorLocation();

    USkeletalMeshComponent* TargetMesh = Target->GetMesh();
    FName AimSocketName = FName("Spine_03");

    if (TargetMesh && TargetMesh->DoesSocketExist(AimSocketName))
    {
        TargetChestLocation = TargetMesh->GetSocketLocation(AimSocketName);
    }
    else
    {
        // bone 못 찾았을 때 기본값
        TargetChestLocation.Z += 50.f;
    }


    // 타겟 방향 바라보기
    AIController->SetFocalPoint(TargetChestLocation);

    // 총구 좌표 및 방향 가져오기
    FVector PawnLocation = OwnerPawn->GetActorLocation();
    // FVector TargetLocation = Target->GetActorLocation();
    FVector TraceStart = PawnLocation + FVector(0, 0, 50.f);
    FVector MuzzleForward = OwnerPawn->GetActorForwardVector();
    UStaticMeshComponent* WeaponMesh = OwnerPawn->FindComponentByClass<UStaticMeshComponent>();
    if (WeaponMesh && WeaponMesh->DoesSocketExist(FName("Muzzle")))
    {
        TraceStart = WeaponMesh->GetSocketLocation(FName("Muzzle"));
    }

    // 사거리 제한 처리
    float DistanceToTarget = FVector::Dist(PawnLocation, TargetChestLocation);
    if (DistanceToTarget > AttackRange + 150.f)
    {
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        UE_LOG(LogTemp, Warning, TEXT("Target out of range. Distance: %f / Range: %f"), DistanceToTarget, AttackRange);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // ---------------------------------------------------------
    // 4. 연사 쿨타임(Fire Rate) 체크
    // ---------------------------------------------------------

    if (Memory->ElapsedTime < AttackInterval)
    {
        return;
    }
#pragma region Aim
    // ---------------------------------------------------------
    // 5. 조준 검사
    // ---------------------------------------------------------

    FVector DirectionToChest = (TargetChestLocation - PawnLocation).GetSafeNormal();

    // 좌우 검사
    FVector MuzzleForward2D = FVector(MuzzleForward.X, MuzzleForward.Y, 0.f).GetSafeNormal();
    FVector DirectionToChest2D = FVector(DirectionToChest.X, DirectionToChest.Y, 0.f).GetSafeNormal();

    if (FVector::DotProduct(MuzzleForward2D, DirectionToChest2D) < 0.9f)
    {
        DrawDebugLine(GetWorld(),TraceStart,TraceStart + (MuzzleForward * 200.f), FColor::Yellow,false,0.1f,0,1.f);
        return;
    }

#pragma endregion

#pragma region Fire
    if (Memory->CurrentAmmo <= 0)
    {
        Memory->bIsReloading = true;
        Memory->ReloadTimer = 0.f;
        UE_LOG(LogTemp, Warning, TEXT("Out of Ammo! Reloading..."));
        return;
    }


    UE_LOG(LogTemp, Error, TEXT("FIRE! AttackInterval: %f / ElapsedTime: %f"), AttackInterval, Memory->ElapsedTime);
    Memory->ElapsedTime = 0.f;
    Memory->CurrentAmmo--;
#pragma endregion

#pragma region attack montage

    // 공격 몽타주 재생
    ALA_AllyAI* AllyAI = Cast<ALA_AllyAI>(OwnerPawn);
    if (AllyAI && AllyAI->AttackMontage)
    {
        if (!AllyAI->bIsFiring)
        {
            AllyAI->bIsFiring = true;

            float MontageLength = AllyAI->PlayAnimMontage(AllyAI->AttackMontage);

            FTimerHandle FireTimerHandle;
            AllyAI->GetWorldTimerManager().SetTimer(FireTimerHandle, AllyAI, &ALA_AllyAI::ResetFiringState, MontageLength, false);
        }
        UE_LOG(LogTemp, Warning, TEXT("몽타주 재생 코드 진입"));
        // AllyAI->PlayAttackMontage();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AllyAI가 NULL이거나 AttackMontage가 비어있음"));
    }

#pragma endregion

#pragma region collision detection
    // ---------------------------------------------------------
    // 7. 충돌 판정 (콜리전 기반 LineTrace)
    // ---------------------------------------------------------
    // 스마트 조준 궤적: 내 총구에서 적 가슴팍 방향으로 쭉 뻗어 나감
    FVector TraceDirection = (TargetChestLocation - TraceStart).GetSafeNormal();
    FVector TraceEnd = TraceStart + (TraceDirection * (DistanceToTarget + 100.f));

    FCollisionQueryParams QueryParams(FName("Attack"), false);
    QueryParams.AddIgnoredActor(OwnerComp.GetOwner());
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredComponent(WeaponMesh);

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);


    // 디버그: 총구 위치와 발사 궤적 그리기
    DrawDebugSphere(GetWorld(), TraceStart, 5.f, 12, FColor::Green, false, 1.f);
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);

    if (bHit)
    {
        if (ALA_BaseCharacter* HitTarget = Cast<ALA_BaseCharacter>(HitResult.GetActor()))
        {
            // 타격 이펙트 및 대미지 적용
            DrawDebugBox(GetWorld(), HitResult.Location, FVector(5.f), FColor::Green, false, 0.2f);
            UGameplayStatics::ApplyDamage(HitTarget, Damage, AIController, OwnerPawn, nullptr);
            UE_LOG(LogTemp, Warning, TEXT("Hit! Ammo Left: %d, Target: %s"), Memory->CurrentAmmo, *HitTarget->GetName());

            // 적 사망 처리
            if (HitTarget->bIsDead)
            {
                // 죽은 적이 자신의 타겟일 때
                if (HitTarget == Target)
                {
                    BlackboardComp->ClearValue(FName("TargetActor"));
                    BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
                    AIController->ClearFocus(EAIFocusPriority::Gameplay);
                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                    return;
                }
            }
        }
        else
        {
            // 환경(벽, 바닥 등)에 맞음
            UE_LOG(LogTemp, Log, TEXT("Attack blocked by environment."));
        }
    }
#pragma endregion
}
