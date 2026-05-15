// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_EnemyFindRRandomPos.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTTask_EnemyFindRRandomPos : public UBTTaskNode
{
    GENERATED_BODY()
public:
    ULA_BTTask_EnemyFindRRandomPos();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPosKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float PatrolRadius = 500.0f;

};
