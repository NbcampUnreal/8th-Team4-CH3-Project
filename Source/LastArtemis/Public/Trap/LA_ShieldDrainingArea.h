// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LA_TrapBase.h"
#include "LA_ShieldDrainingArea.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_ShieldDrainingArea : public ALA_TrapBase
{
    GENERATED_BODY()

public:
    ALA_ShieldDrainingArea();

protected:
    // 틱마다 깎을 쉴드 양
    UPROPERTY(EditAnywhere, Category = "Settings")
    float ShieldDrainAmount;

    // 베이스의 가상 함수 오버라이드
    virtual void ApplyEffect(AActor* TargetActor) override;
};
