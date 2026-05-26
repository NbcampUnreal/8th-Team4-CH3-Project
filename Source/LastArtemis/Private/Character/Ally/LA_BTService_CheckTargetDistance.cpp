// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Ally/LA_BTService_CheckTargetDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ULA_BTService_CheckTargetDistance::ULA_BTService_CheckTargetDistance()
{
    NodeName = TEXT("Check Target Distance (Aggro)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    LoseSightDistance = 5000.f;

    bCreateNodeInstance = true;
}

void ULA_BTService_CheckTargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!AIController || !OwnerPawn || !Blackboard) return;

    // 현재 타겟 가져오기
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(FName("TargetActor")));
    bool bIsCommandedTarget = Blackboard->GetValueAsBool(FName("IsCommandedTarget"));

    // 기존 타겟 거리 검사 및 놓침 처리용 변수
    float CurrentTargetDistanceSq = 99999999.f;
    float LoseSightDistSq = FMath::Square(LoseSightDistance);

    // 기존 타겟이 존재할 때 처리
    if (Target)
    {
        CurrentTargetDistanceSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Target->GetActorLocation());

        ALA_BaseCharacter* TargetBase = Cast<ALA_BaseCharacter>(Target);

        // 🎯 1. [심볼 에러 수정]
        // 직접 bIsDead를 조준하지 않고 적의 헬스 컴포넌트를 통해 숨이 끊어졌는지 안전 검사합니다.
        bool bIsTargetDead = false;
        if (TargetBase && TargetBase->GetHealthComponent())
        {
            bIsTargetDead = TargetBase->GetHealthComponent()->IsDead();
        }

        if (CurrentTargetDistanceSq > LoseSightDistSq || bIsTargetDead)
        {
            Blackboard->ClearValue(FName("TargetActor"));
            Blackboard->SetValueAsBool(FName("IsCommandedTarget"), false);
            AIController->ClearFocus(EAIFocusPriority::Gameplay);
            AIController->ClearFocus(EAIFocusPriority::Move);
            AIController->ClearFocus(EAIFocusPriority::Default);
            Target = nullptr;
            CurrentTargetDistanceSq = 99999999.f;
            UE_LOG(LogTemp, Warning, TEXT("적 타겟을 놓쳤습니다! 거리가 너무 멉니다 혹은 사망했습니다."));
        }
    }

    // 명령받은 타겟이 살아있으면 탐색하지 않고 스킵
    if (bIsCommandedTarget && Target != nullptr)
    {
        return;
    }

    // 타겟이 존재하지 않거나 자유 탐색 상태일 때 새로운 최적의 타겟 찾기
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_BaseCharacter::StaticClass(), AllEnemies);

    AActor* BestTarget = Target;
    float MinDistSq = CurrentTargetDistanceSq;
    float SwitchThresholdSq = FMath::Square(150.f); // 타겟 전환 버퍼 마진

    for (AActor* Actor : AllEnemies)
    {
        if (Actor == OwnerPawn) continue;

        ALA_BaseCharacter* Enemy = Cast<ALA_BaseCharacter>(Actor);

        // 🎯 2. [심볼 에러 수정] 루프 내의 주변 적 탐색 시에도 컴포넌트의 IsDead()로 필터링합니다.
        if (!Enemy || (Enemy->GetHealthComponent() && Enemy->GetHealthComponent()->IsDead())) continue;

        // 🎯 3. [오타 버그 완벽 교정] Team.Team.Ally -> Team.Ally로 수정하여 오사 및 팀킬을 원천 차단합니다!
        if (Enemy->CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")))) continue;

        float DistToEnemySq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Enemy->GetActorLocation());

        // 거리가 시야 반경 내일 때 추가 검사
        if (DistToEnemySq < LoseSightDistSq)
        {
            // AI의 눈에 보이는 위치인지 검사
            FHitResult HitResult;
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(OwnerPawn);

            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                OwnerPawn->GetActorLocation(),
                Enemy->GetActorLocation(),
                ECC_Visibility,
                CollisionParams
            );

            bool bHasLineOfSight = bHit && (HitResult.GetActor() == Enemy);

            if (bHasLineOfSight)
            {
                if (Target == nullptr || DistToEnemySq < (MinDistSq - SwitchThresholdSq))
                {
                    MinDistSq = DistToEnemySq;
                    BestTarget = Enemy;
                }
            }


        }
    }

    if (BestTarget != nullptr && BestTarget != Target)
    {
        Blackboard->SetValueAsObject(FName("TargetActor"), BestTarget);
    }
}
