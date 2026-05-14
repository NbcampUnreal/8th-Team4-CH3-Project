// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LA_GameInstance.generated.h"

class ULA_MissionDataAsset;

UCLASS(Blueprintable)
class LASTARTEMIS_API ULA_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	ULA_GameInstance();

public:

    ////////////////////////////////////////////////////////////////////////
    /// 옵션 설정값 저장
    ///////////////////////////////////////////////////////////////////////
    virtual void Init() override;

    // 설정을 바꿀 때마다 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void UpdateAndSaveSettings(EMovementInputMode NewAimMode, EMovementInputMode NewSprintMode);

    // 캐릭터에게 설정 적용
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplySettingsToCharacter();

    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EMovementInputMode CurrentAimInputMode;

    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EMovementInputMode CurrentSprintInputMode;

    // 설정 전용 슬롯
    const FString SettingsSlotName = TEXT("UserSettings");

private:
    void LoadSettingsFromDisk();

public:

    ////////////////////////////////////////////////////////////////////////
    /// 보상 로직 함수
    ////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, Category = "Reward")
    void AddReward(int32 GoldReward, int32 ScoreReward);
	// 게임 데이터 초기화
	UFUNCTION(BlueprintCallable)
	void ResetPlayerData();

    ////////////////////////////////////////////////////////////////////////
    /// 미션 로직 함수
    ////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetSelectedMission(ULA_MissionDataAsset* InMissionData);
    UFUNCTION(BlueprintPure, Category = "Mission")
    ULA_MissionDataAsset* GetSelectedMission() const;

    ////////////////////////////////////////////////////////////////////////
    /// 세이브 로직 함수
    ////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveCheckPointData(int32 PhaseIndex, FVector SaveLocation, FRotator SaveRotation, int32 ElapsedGameTime);

    // 게임 데이터 저장
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveGameData();

    // 게임 데이터 불러오기
    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadGameData();


public:
    // 현재 골드
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 TotalGold;
    // 몬스터 처치 점수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 TotalScore;


    // 선택된 미션 Data Asset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    ULA_MissionDataAsset* SelectedMissionDataAsset;


    // 최근 방문한 Rest Stage의 Phase Index
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    int32 SavedPhaseIndex;

    // Check Point와 상호 작용한 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FVector SavedPlayerLocation;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FRotator SavedPlayerRotation;

    // 지금까지 플레이한 시간
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    int32 SavedElapsedGameTime;

    // 기본 슬롯 이름
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save Data")
    FString SaveSlotName;

    // 저장됐는지
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
    bool bSaveSuccess;
};
