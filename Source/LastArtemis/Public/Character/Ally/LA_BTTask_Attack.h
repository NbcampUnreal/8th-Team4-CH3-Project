#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "LA_BTTask_Attack.generated.h"


UCLASS()
class LASTARTEMIS_API ULA_BTTask_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
public:
    // 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRange = 200.f;
    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackPower = 10.f;

};
