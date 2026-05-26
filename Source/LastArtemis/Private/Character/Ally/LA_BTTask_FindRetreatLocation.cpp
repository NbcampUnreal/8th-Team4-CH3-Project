// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_FindRetreatLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Character/Ally/LA_AllyAIController.h"

ULA_BTTask_FindRetreatLocation::ULA_BTTask_FindRetreatLocation()
{
    NodeName = TEXT("Find Retreat Location");
    DangerRadius = 400.f;
    RetreatDistance = 600.f;
}

EBTNodeResult::Type ULA_BTTask_FindRetreatLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!AIController || !Blackboard) return EBTNodeResult::Failed;

    APawn* OwnerPawn = AIController->GetPawn();
    if (!OwnerPawn) return EBTNodeResult::Failed;

    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!IsValid(TargetActor)) return EBTNodeResult::Failed;

    FVector AILocation = OwnerPawn->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();

    // 적과의 거리 계산
    float DistanceToTarget = FVector::Dist(AILocation, TargetLocation);

    if (DistanceToTarget > DangerRadius)
    {
        return EBTNodeResult::Failed;
    }

    // 적 반대 방향 벡터 구하기
    FVector DirectionAway = (AILocation - TargetLocation). GetSafeNormal();
    DirectionAway.Z = 0.f;

    FVector TargetRetreatLocation = AILocation + (DirectionAway * RetreatDistance);

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerComp.GetWorld());
    FNavLocation SafeNavLocation;

    if (NavSystem && NavSystem->ProjectPointToNavigation(TargetLocation, SafeNavLocation, FVector(500.f, 500.f, 500.f)))
    {
        Blackboard->SetValueAsVector(BlackboardKey.SelectedKeyName, SafeNavLocation.Location);
        UE_LOG(LogTemp, Display, TEXT("아군 AI: 회피 발동! 도망 완료"));
        return EBTNodeResult::Succeeded;
    }
    UE_LOG(LogTemp, Error, TEXT("아군 AI: 뒤가 벽으로 막힘. 회피 불가!"));
    return EBTNodeResult::Failed;
}
