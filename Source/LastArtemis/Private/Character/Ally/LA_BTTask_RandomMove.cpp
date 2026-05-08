// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_RandomMove.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"

ULA_BTTask_RandomMove::ULA_BTTask_RandomMove()
{
    NodeName = TEXT("Random Move");
    WanderRadius = 500.f;
    MinDistance = 300.f;
}

EBTNodeResult::Type ULA_BTTask_RandomMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* AIController = OwnerComp.GetAIOwner())
    {
        if (APawn* OwnerPawn = AIController->GetPawn())
        {
            if (AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor"))))
            {
                FNavLocation RandomLocation;
                UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
                if (NavSystem && NavSystem->GetRandomReachablePointInRadius(Player->GetActorLocation(), WanderRadius, RandomLocation))
                {
                    if (FVector::Dist(RandomLocation.Location, Player->GetActorLocation()) < MinDistance)
                    {
                        return EBTNodeResult::Failed;
                    }
                    else
                    {
                        AIController->MoveToLocation(RandomLocation.Location);
                        UE_LOG(LogTemp, Warning, TEXT("RandomMove Succeeded"));
                        return EBTNodeResult::Succeeded;
                    }

                }
            }
        }
    }
    

    return EBTNodeResult::Failed;

}
