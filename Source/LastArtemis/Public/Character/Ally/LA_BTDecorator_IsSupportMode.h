// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "LA_BTDecorator_IsSupportMode.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTDecorator_IsSupportMode : public UBTDecorator_Blackboard
{
	GENERATED_BODY()

public:
    ULA_BTDecorator_IsSupportMode();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
