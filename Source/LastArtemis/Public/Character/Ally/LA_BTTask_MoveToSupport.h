#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_MoveToSupport.generated.h"

UCLASS()
class LASTARTEMIS_API ULA_BTTask_MoveToSupport : public UBTTaskNode
{
	GENERATED_BODY()

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float BehindDistance = 150.f;

};
