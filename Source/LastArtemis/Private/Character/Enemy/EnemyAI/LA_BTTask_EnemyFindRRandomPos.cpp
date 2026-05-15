#include "Character/Enemy/EnemyAI/LA_BTTask_EnemyFindRRandomPos.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

ULA_BTTask_EnemyFindRRandomPos::ULA_BTTask_EnemyFindRRandomPos()
{
    NodeName = TEXT("FindRandomPos");
}

EBTNodeResult::Type ULA_BTTask_EnemyFindRRandomPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControllingPawn) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());;
    if (!NavSystem) return EBTNodeResult::Failed;

    FNavLocation NextPos;
    if (NavSystem->GetRandomPointInNavigableRadius(ControllingPawn->GetActorLocation(), PatrolRadius, NextPos))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetPosKey.SelectedKeyName, NextPos.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
