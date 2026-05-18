#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "LA_BTTask_FindPatrolLocation.generated.h"

UCLASS()
class LASTARTEMIS_API ULA_BTTask_FindPatrolLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
    ULA_BTTask_FindPatrolLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float WanderRadius;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float BehindDistance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float SpreadDistance = 250.f;
};
