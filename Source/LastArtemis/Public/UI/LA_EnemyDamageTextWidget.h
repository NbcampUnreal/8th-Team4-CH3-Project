// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_EnemyDamageTextWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_EnemyDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    AActor* ParentActor;

    // 블루프린트에서 구현할 대미지 설정 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void SetDamageValue(float Damage);

};
