// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LA_TrapBase.h"
#include "LA_ContaminationPad.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_ContaminationPad : public ALA_TrapBase
{
    GENERATED_BODY()

public:
    ALA_ContaminationPad();

protected:
    // 틱마다 증가시킬 오염도 양
    UPROPERTY(EditAnywhere, Category = "Settings")
    float ContaminationAmount;

    // 베이스의 가상 함수를 오버라이드하여 실제 로직 구현
    virtual void ApplyEffect(AActor* TargetActor) override;
};
