// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_MoveToSupport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Navigation/PathFollowingComponent.h"

ULA_BTTask_MoveToSupport::ULA_BTTask_MoveToSupport()
{
    bNotifyTick = true;
    NodeName = TEXT("Move To Support");
}

EBTNodeResult::Type ULA_BTTask_MoveToSupport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    if (!AIController) return EBTNodeResult::Failed;
    ALA_PlayerCharacter* Player = Cast<ALA_PlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("PlayerActor")));
    if (!Player) return EBTNodeResult::Failed;

    /*AIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
    AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject
    (
      this,
      &ULA_BTTask_MoveToSupport::OnMoveCompleted
    );*/

    // 플레이어의 등 뒤 좌표 계산
    FVector PlayerForward = Player->GetActorForwardVector();
    FVector SupportLocation = Player->GetActorLocation() - PlayerForward * BehindDistance;

    // 이동 명령
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(SupportLocation);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

    FPathFollowingRequestResult RequestResult = AIController->MoveTo(MoveRequest);
    if (RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        UE_LOG(LogTemp, Log, TEXT("Move To Support : Already At Goal"));
        return EBTNodeResult::Succeeded;
    }
    else if (RequestResult.Code == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Warning, TEXT("Move To Support Started"))

    // MoveRequestID = RequestResult.MoveId;
    return EBTNodeResult::InProgress;

}

void ULA_BTTask_MoveToSupport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(OwnerComp.GetAIOwner());
    if (!AIController) return;

    EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        // Idle 상태
        UE_LOG(LogTemp, Log, TEXT("MoveToSupport : Succeeded"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
    else if (MoveStatus == EPathFollowingStatus::Waiting)
    {
        // 이동 실패 혹은 막힘
        UE_LOG(LogTemp, Warning, TEXT("MoveToSupport : Waiting"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }

    // Moving 상태
}

/*void ULA_BTTask_MoveToSupport::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (OwnerCompRef && RequestID == MoveRequestID)
    {
        FinishLatentTask(*OwnerCompRef, EBTNodeResult::Succeeded);
    }
}*/



