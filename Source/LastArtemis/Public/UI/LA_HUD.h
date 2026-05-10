// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_HUD.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_HUD : public UUserWidget
{
	GENERATED_BODY()

public:

    // UMG 위젯이 화면에 “실제로 생성되고 붙을 때” 호출되는 C++ 초기화 함수
    virtual void NativeConstruct() override;


    // 블루프린트에서 구현할 함수
    UFUNCTION(BlueprintImplementableEvent)
    void UpdateHP(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateShield(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateAmmo(int32 CurrentMagazineAmmo, int32 MaxMagazineSize);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateMission();



    void BindHealth();

    void BindAmmo();


};
