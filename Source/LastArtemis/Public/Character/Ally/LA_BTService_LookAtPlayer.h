// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "LA_BTService_LookAtPlayer.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ULA_BTService_LookAtPlayer : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
    ULA_BTService_LookAtPlayer();

    // 보간 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    float InterpSpeed;

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)  override;
};
