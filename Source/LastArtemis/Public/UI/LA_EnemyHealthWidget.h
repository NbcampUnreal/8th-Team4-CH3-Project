// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_EnemyHealthWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_EnemyHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    void UpdateHealthBar(float Current, float Max)
    {
        if (Max > 0.0f)
        {
            float Percent = Current / Max;

            // HealthBar 수치 변경
            NativeUpdateHealthBar(Percent);
        }
    }

protected:
    // 블루프린트 HealthBar 업데이트
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void NativeUpdateHealthBar(float Percent);
};
