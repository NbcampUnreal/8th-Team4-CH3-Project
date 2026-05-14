// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_MoveToSupport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

EBTNodeResult::Type ULA_BTTask_MoveToSupport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* AIController = OwnerComp.GetAIOwner())
    {
        if (AActor* Player = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor"))))
        {
            OwnerCompRef = &OwnerComp;

            AIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
            AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject
            (
              this,
              &ULA_BTTask_MoveToSupport::OnMoveCompleted
            );

            FVector PlayerForward = Player->GetActorForwardVector();
            FVector SupportLocation = Player->GetActorLocation() - PlayerForward * BehindDistance;

            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(SupportLocation);
            MoveRequest.SetAcceptanceRadius(50.f);

            FPathFollowingRequestResult RequestResult = AIController->MoveTo(MoveRequest);
            MoveRequestID = RequestResult.MoveId;


            return EBTNodeResult::InProgress;
        }
    }
    return EBTNodeResult::Failed;
}

void ULA_BTTask_MoveToSupport::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (OwnerCompRef && RequestID == MoveRequestID)
    {
        FinishLatentTask(*OwnerCompRef, EBTNodeResult::Succeeded);
    }
}



