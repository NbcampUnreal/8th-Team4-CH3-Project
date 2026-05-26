// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Ally/LA_BTTask_Attack.h"
#include "Character/LA_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Component/LA_HealthComponent.h"

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
    // 🎯 [심볼 에러 및 복사 버그 수정]
    // 직접 Target->bIsDead를 조준하지 않고, 부모의 헬스 컴포넌트를 관통하여 적의 사망 여부를 판단합니다.
    else if (Target->GetHealthComponent() && Target->GetHealthComponent()->IsDead())
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 중단 원인 3: 적의 체력이 다 되어 사망(IsDead) 상태입니다! (Succeeded 종료)"));
        BlackboardComp->ClearValue(FName("TargetActor"));
        BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

#pragma region reload
    if (Memory->bIsReloading)
    {
        Memory->ReloadTimer += DeltaSeconds;
        if (Memory->ReloadTimer >= 2.0f)
        {
            Memory->CurrentAmmo = TotalAmmo;
            Memory->bIsReloading = false;
            Memory->ReloadTimer = 0.f;
            UE_LOG(LogTemp, Warning, TEXT("Reload Complete!"));
        }
        return;
    }
#pragma endregion

    // ---------------------------------------------------------
    // 2. 적의 가슴팍 위치 추적 및 회전
    // ---------------------------------------------------------
    FVector TargetChestLocation = Target->GetActorLocation();
    USkeletalMeshComponent* TargetMesh = Target->GetMesh();
    FName AimSocketName = FName("Spine_03");

    // 💡 스태틱 메시 보스나 터렛은 스켈레탈 소켓이 없을 수 있으므로 안전 검사 수행
    if (TargetMesh && TargetMesh->DoesSocketExist(AimSocketName))
    {
        TargetChestLocation = TargetMesh->GetSocketLocation(AimSocketName);
    }
    else
    {
        TargetChestLocation.Z += 50.f;
    }

    AIController->SetFocalPoint(TargetChestLocation);

    FVector PawnLocation = OwnerPawn->GetActorLocation();
    FVector TraceStart = PawnLocation + FVector(0, 0, 50.f);
    FVector MuzzleForward = OwnerPawn->GetActorForwardVector();
    UStaticMeshComponent* WeaponMesh = OwnerPawn->FindComponentByClass<UStaticMeshComponent>();
    if (WeaponMesh && WeaponMesh->DoesSocketExist(FName("Muzzle")))
    {
        TraceStart = WeaponMesh->GetSocketLocation(FName("Muzzle"));
    }

    float DistanceToTarget = FVector::Dist(PawnLocation, TargetChestLocation);
    if (DistanceToTarget > AttackRange + 150.f)
    {
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        UE_LOG(LogTemp, Warning, TEXT("Target out of range. Distance: %f / Range: %f"), DistanceToTarget, AttackRange);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    if (Memory->ElapsedTime < AttackInterval)
    {
        return;
    }

#pragma region Aim
    FVector DirectionToChest = (TargetChestLocation - PawnLocation).GetSafeNormal();
    FVector MuzzleForward2D = FVector(MuzzleForward.X, MuzzleForward.Y, 0.f).GetSafeNormal();
    FVector DirectionToChest2D = FVector(DirectionToChest.X, DirectionToChest.Y, 0.f).GetSafeNormal();

    if (FVector::DotProduct(MuzzleForward2D, DirectionToChest2D) < 0.9f)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceStart + (MuzzleForward * 200.f), FColor::Yellow, false, 0.1f, 0, 1.f);
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
    }
#pragma endregion

#pragma region collision detection
    FVector TraceDirection = (TargetChestLocation - TraceStart).GetSafeNormal();
    FVector TraceEnd = TraceStart + (TraceDirection * (DistanceToTarget + 100.f));

    FCollisionQueryParams QueryParams(FName("Attack"), false);
    QueryParams.AddIgnoredActor(OwnerComp.GetOwner());
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredComponent(WeaponMesh);

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

    DrawDebugSphere(GetWorld(), TraceStart, 5.f, 12, FColor::Green, false, 1.f);
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);

    if (bHit)
    {
        if (ALA_BaseCharacter* HitTarget = Cast<ALA_BaseCharacter>(HitResult.GetActor()))
        {
            DrawDebugBox(GetWorld(), HitResult.Location, FVector(5.f), FColor::Green, false, 0.2f);
            UGameplayStatics::ApplyDamage(HitTarget, Damage, AIController, OwnerPawn, nullptr);

            // 🎯 3. [타격 타겟 사망 감지 교정]
            // 레이캐스트에 맞은 적이 진짜 숨이 끊어졌는지도 헬스 컴포넌트를 조준해 감지합니다.
            if (HitTarget->GetHealthComponent() && HitTarget->GetHealthComponent()->IsDead())
            {
                if (HitTarget == Target)
                {
                    BlackboardComp->ClearValue(FName("TargetActor"));
                    BlackboardComp->SetValueAsBool(FName("IsCommandedTarget"), false);
                    AIController->ClearFocus(EAIFocusPriority::Gameplay);
                    AIController->ClearFocus(EAIFocusPriority::Default);
                    AIController->ClearFocus(EAIFocusPriority::Move);
                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                    return;
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Attack blocked by environment."));
        }
    }
#pragma endregion
}
