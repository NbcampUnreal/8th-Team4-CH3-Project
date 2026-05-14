// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "LA_BTService_CheckState.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTService_CheckState : public UBTService_BlackboardBase
{
	GENERATED_BODY()


public:
    ULA_BTService_CheckState();

    void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);

};
