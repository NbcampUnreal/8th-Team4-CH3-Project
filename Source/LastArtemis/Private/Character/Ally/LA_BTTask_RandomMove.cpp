// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_RandomMove.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"

ULA_BTTask_RandomMove::ULA_BTTask_RandomMove()
{
    NodeName = TEXT("Random Move");

    BehindDistance = 300.f;
    WanderRadius = 300.f;
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
                // 플레이어 뒤쪽 기준점 계산
                FVector PlayerBackOffset = Player->GetActorLocation()
                - (Player->GetActorForwardVector() * BehindDistance);


                float RandomAngle = FMath::RandRange(-45.f, 45.f);
                FVector PatrolLocation = PlayerBackOffset.RotateAngleAxis(RandomAngle, FVector::UpVector);

                // NavMesh 위 유효한 위치 찾기
                FNavLocation NavLocation;
                UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
                if (NavSystem && NavSystem->ProjectPointToNavigation(PatrolLocation, NavLocation))
                {
                    AIController->MoveToLocation(NavLocation.Location);
                    UE_LOG(LogTemp, Warning, TEXT("PlayerFollow Succeeded"));
                    return EBTNodeResult::Succeeded;

                    /*if (FVector::Dist(RandomLocation.Location, Player->GetActorLocation()) < MinDistance)
                    {
                        return EBTNodeResult::Failed;
                    }
                    else
                    {


                        AIController->MoveToLocation(RandomLocation.Location);
                        UE_LOG(LogTemp, Warning, TEXT("RandomMove Succeeded"));
                        return EBTNodeResult::Succeeded;
                    }*/

                }
            }
        }
    }


    return EBTNodeResult::Failed;

}
