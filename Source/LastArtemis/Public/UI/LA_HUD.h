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

    // 아군 체력 업데이트용 C++ 함수 (Delegate 연결용)
    void UpdateAlly1HP(float Current, float Max);
    void UpdateAlly2HP(float Current, float Max);

    // 블루프린트에서 세 개의 바 비율을 받아 처리하는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void NativeUpdateAlly1HealthBar(float HealthPercent, float DamagePercent, float SpacePercent);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void NativeUpdateAlly2HealthBar(float HealthPercent, float DamagePercent, float SpacePercent);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateShield(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateContamination(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateSkill(int32 Index, float CooldownTime);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateQuickSlot(UTexture2D* ItemIcon, int32 Quantity);

    UPROPERTY(BlueprintReadOnly)
    class ULA_InventoryComponent* InventoryComp;

    UFUNCTION()
    void HandleQuickSlotUpdated(int32 QuickSlotIndex);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateAmmo(int32 CurrentMagazineAmmo, int32 MaxMagazineSize);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateMission(ULA_MissionDataAsset* MissionData, int32 PhaseIndex, int32 CurrentCount);

    UFUNCTION(BlueprintImplementableEvent)
    void UpdateWeapon(ULA_WeaponData* WeaponData);

    void RegisterAllyAuto(class ALA_AllyAI* NewAlly);

    void BindHealth();

    void BindContamination();

    void BindSkill();

    void BindQuickSlot();

    void BindAmmo();

    void BindWeapon();

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

private:
    // 아군 1 체력 정보
    float Ally1CurrentHealth = 0.0f;
    float Ally1MaxHealth = 0.0f;
    float Ally1DelayedHealth = 0.0f;

    // 아군 2 체력 정보
    float Ally2CurrentHealth = 0.0f;
    float Ally2MaxHealth = 0.0f;
    float Ally2DelayedHealth = 0.0f;

    // 보간 속도
    UPROPERTY(EditAnywhere, Category = "UI")
    float InterpSpeed = 10.0f;

    void UpdateAllyRatios(int32 AllyIndex);

};
