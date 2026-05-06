// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "LA_BTDecorator_SupportCondition.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_BTDecorator_SupportCondition : public UBTDecorator
{
	GENERATED_BODY()

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
