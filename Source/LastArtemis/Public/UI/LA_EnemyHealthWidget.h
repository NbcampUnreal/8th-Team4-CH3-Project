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
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // 체력 업데이트 (상태 변경 시 호출)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateHealthBar(float Current, float Max);

protected:
    // 블루프린트에서 세 개의 바 비율을 받아 처리하는 이벤트
    // HealthPercent: 실제 체력 (빨간색)
    // DamagePercent: 깎인 체력 표시 (흰색/노란색)
    // SpacePercent: 비어있는 공간
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void NativeUpdateHealthBar(float HealthPercent, float DamagePercent, float SpacePercent);

    // 내부 비율 계산 및 이벤트 호출
    void UpdateRatios();

private:
    float CurrentHealth = 0.0f;
    float MaxHealth = 0.0f;
    float DelayedHealth = 0.0f;

    // 데미지 바 감소 속도
    UPROPERTY(EditAnywhere, Category = "UI")
    float InterpSpeed = 10.0f;
};
