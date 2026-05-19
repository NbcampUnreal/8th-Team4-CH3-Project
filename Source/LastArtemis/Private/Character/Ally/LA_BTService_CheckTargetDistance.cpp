// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTService_CheckTargetDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"

ULA_BTService_CheckTargetDistance::ULA_BTService_CheckTargetDistance()
{
    NodeName = TEXT("Check Target Distance (Aggro)");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    LoseSightDistance = 1500.f;

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

    UE_LOG(LogTemp, Display, TEXT("CheckTargetDistance Service Ticking"));
    // 현재 타겟 가져오기
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(FName("TargetActor")));
    if (Target)
    {
        bool bIsCommandedTarget = Blackboard->GetValueAsBool(FName("IsCommandedTarget"));
        UE_LOG(LogTemp, Warning, TEXT("IsCommandedTarget: %s"), bIsCommandedTarget ? TEXT("true") : TEXT("false"));

        if (bIsCommandedTarget)
        {
            return;
        }


        // 기존 타겟 거리 검사 및 놓침 처리
        float CurrentTargetDistanceSq = 99999999.f;
        float LoseSightDistSq = FMath::Square(LoseSightDistance);

        if (Target)
        {
            CurrentTargetDistanceSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Target->GetActorLocation());

            if (CurrentTargetDistanceSq > LoseSightDistSq)
            {
                Blackboard->ClearValue(FName("TargetActor"));
                AIController->ClearFocus(EAIFocusPriority::Gameplay);
                AIController->ClearFocus(EAIFocusPriority::Move);
                AIController->ClearFocus(EAIFocusPriority::Default);
                Target = nullptr;
                CurrentTargetDistanceSq = 99999999.f;
                UE_LOG(LogTemp, Warning, TEXT("적 타겟을 놓쳤습니다! 거리가 너무 멉니다."));
            }
        }

        TArray<ALA_EnemyCharacter*> AllEnemies;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_EnemyCharacter::StaticClass(), AllEnemies);

        ALA_EnemyCharacter* BestTarget = Target;
        float MinDistSq = CurrentTargetDistanceSq;

        // 타겟 전환에 필요한 최소 거리 차이
        float SwitchThresholdSq = FMath::Square(150.f);

        for (ALA_EnemyCharacter* Enemy : AllEnemies)
        {
            if (Enemy->bIsDead) continue;

            float DistToEnemySq = FVector::DistSquared(OwnerPawn->GetActorLocation(), Enemy->GetActorLocation());

            if (DistToEnemySq < LoseSightDistSq)
            {
                if (Target == nullptr)
                {
                    if (DistToEnemySq < MinDistSq)
                    {
                        MinDistSq = DistToEnemySq;
                        BestTarget = Enemy;
                    }
                }
            }
        }
    }

}


