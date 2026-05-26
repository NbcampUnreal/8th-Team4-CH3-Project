// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "LA_BTTask_FindRetreatLocation.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTTask_FindRetreatLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
    ULA_BTTask_FindRetreatLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, Category = "Retreat")
    float DangerRadius;
    UPROPERTY(EditAnywhere, Category = "Retreat")
    float RetreatDistance;
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

};
