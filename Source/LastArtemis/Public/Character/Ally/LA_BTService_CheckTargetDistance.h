// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "LA_BTService_CheckTargetDistance.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTService_CheckTargetDistance : public UBTService
{
	GENERATED_BODY()
private:
    ULA_BTService_CheckTargetDistance();


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float LoseSightDistance;

    void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);

};
