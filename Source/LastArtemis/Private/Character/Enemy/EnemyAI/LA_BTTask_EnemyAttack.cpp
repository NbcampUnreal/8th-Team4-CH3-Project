#include "Character/Enemy/EnemyAI/LA_BTTask_EnemyAttack.h"
#include "AIController.h"
#include "Character/Enemy/LA_EnemyCharacter.h"

ULA_BTTask_EnemyAttack::ULA_BTTask_EnemyAttack()
{
        NodeName = TEXT("Attack");
}

EBTNodeResult::Type ULA_BTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller)return EBTNodeResult::Failed;

    ALA_EnemyCharacter* Enemy = Cast<ALA_EnemyCharacter>(Controller->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    Enemy->PlayAttackMontage();

    return EBTNodeResult::Succeeded;
}
