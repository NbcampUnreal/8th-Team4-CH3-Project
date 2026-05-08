// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_MoveToSupport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

EBTNodeResult::Type ULA_BTTask_MoveToSupport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* AIController = OwnerComp.GetAIOwner())
    {
        if (APawn* OwnerPawn = AIController->GetPawn())
        {
            if (AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor"))))
            {
                FVector PlayerForawrd = Player->GetActorForwardVector();
                FVector SupportLocation = Player->GetActorLocation() - PlayerForawrd * BehindDistance;
                AIController->MoveToLocation(SupportLocation);
                return EBTNodeResult::Succeeded;
            }
        }
    }
    return EBTNodeResult::Failed;
}
