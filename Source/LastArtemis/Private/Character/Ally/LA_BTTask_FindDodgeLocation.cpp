// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_FindDodgeLocation.h"

#include "NavigationSystem.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ULA_BTTask_FindDodgeLocation::ULA_BTTask_FindDodgeLocation()
{
    NodeName = TEXT("Find Dodge Location");
    DodgeRadius = 300.f;
}

EBTNodeResult::Type ULA_BTTask_FindDodgeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp) return EBTNodeResult::Failed;

    ALA_AllyAI* AllyCharacter = Cast<ALA_AllyAI>(AIController->GetPawn());
    if (!AllyCharacter) return EBTNodeResult::Failed;

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerComp.GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FVector TargetLocation = TargetActor->GetActorLocation();
    FNavLocation RandomNavLocation;

    if (NavSystem->GetRandomReachablePointInRadius(TargetLocation, DodgeRadius, RandomNavLocation))
    {
        BlackboardComp->SetValueAsVector(DodgeLocationKey.SelectedKeyName, RandomNavLocation.Location);

        UE_LOG(LogTemp, Warning, TEXT("AllyAI: 회피 위치 탐색 성공"));

        return EBTNodeResult::Succeeded;

    }

    return EBTNodeResult::Failed;
}


