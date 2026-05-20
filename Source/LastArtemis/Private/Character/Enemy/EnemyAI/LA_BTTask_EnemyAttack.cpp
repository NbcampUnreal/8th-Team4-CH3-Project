#include "Character/Enemy/EnemyAI/LA_BTTask_EnemyAttack.h"
#include "AIController.h"
#include "Character/Enemy/LA_EnemyCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ULA_BTTask_EnemyAttack::ULA_BTTask_EnemyAttack()
{
    NodeName = TEXT("Attack");
    bNotifyTick = true;
}

EBTNodeResult::Type ULA_BTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    ALA_EnemyCharacter* Enemy = Cast<ALA_EnemyCharacter>(Controller->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    // [수정] OwnerComp 주소와 함께, 현재 실행 중인 테스크 자신인 'this'를 매개변수로 같이 넘깁니다!
    Enemy->PlayAttackMontageWithComp(&OwnerComp, this);

    return EBTNodeResult::InProgress;
}

void ULA_BTTask_EnemyAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    if (Controller)
    {
        ALA_EnemyCharacter* Enemy = Cast<ALA_EnemyCharacter>(Controller->GetPawn());

        if (Enemy && !Enemy->IsAttacking())
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}
