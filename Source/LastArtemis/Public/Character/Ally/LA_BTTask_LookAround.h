// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LA_BTTask_LookAround.generated.h"

/*
 *작성자 : 안나영
 */

struct FLookAroundMemory
{
    float ElapsedTime = 0.f;
    // 목표 회전값
    float TargetYaw = 0.f;
    // 방향
    bool bTurningRight = false;
};

UCLASS()
class LASTARTEMIS_API ULA_BTTask_LookAround : public UBTTaskNode
{
	GENERATED_BODY()

public:
    ULA_BTTask_LookAround();

    //초당 회전 각도
    UPROPERTY(EditAnywhere, Category = "Properties")
    float RotationSpeed = 60.f;
    //응시 지속 시간
    UPROPERTY(EditAnywhere, Category = "Properties")
    float LookDuration = 2.0f;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FLookAroundMemory);
    }
};
