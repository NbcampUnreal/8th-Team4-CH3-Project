// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_FindDodgeLocation.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTTask_FindDodgeLocation : public UBTTaskNode
{
	GENERATED_BODY()

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    ULA_BTTask_FindDodgeLocation();

    UPROPERTY(EditAnywhere, Category = "Evade")
    float DodgeRadius;
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DodgeLocationKey;


};
