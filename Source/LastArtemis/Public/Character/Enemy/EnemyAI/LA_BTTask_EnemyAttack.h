#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_EnemyAttack.generated.h"


UCLASS()
class LASTARTEMIS_API ULA_BTTask_EnemyAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:

    ULA_BTTask_EnemyAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);
};
