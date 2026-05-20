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
    void UpdateAlly1HP(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateAlly2HP(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateShield(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateContamination(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateQuickSlot(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateAmmo(int32 CurrentMagazineAmmo, int32 MaxMagazineSize);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateMission(ULA_MissionDataAsset* MissionData, int32 PhaseIndex, int32 CurrentCount);

    void RegisterAllyAuto(class ALA_AllyAI* NewAlly);

    void BindHealth();

    void BindContamination();

    void BindQuickSlot();

    void BindAmmo();

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Reference")
    class ALA_GameStateBase* GameState;

    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_Ally1;

    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_Ally2;
        
    // 등록된 아군 액터
    UPROPERTY()
    TObjectPtr<AActor> Ally1Actor;

    UPROPERTY()
    TObjectPtr<AActor> Ally2Actor;

};
