#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_MoveToSupport.generated.h"

struct FPathFollowingResult;
class UBehaviorTreeComponent;

UCLASS()
class LASTARTEMIS_API ULA_BTTask_MoveToSupport : public UBTTaskNode
{
	GENERATED_BODY()

protected:

    ULA_BTTask_MoveToSupport();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    // void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float BehindDistance = 150.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    float AcceptanceRadius = 100.f;

    FAIRequestID MoveRequestID;
};
