// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameMode/LA_SaveGame.h"
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
    /// 점수 로직
    ////////////////////////////////////////////////////////////////////////
    
    // 점수 증가
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddScore(int32 ScoreAmount);

    // 점수 초기화
    UFUNCTION(BlueprintCallable, Category = "Score")
    void ResetScore();

    UFUNCTION(BlueprintCallable, Category = "Score")
    int32 GetScore();

    ////////////////////////////////////////////////////////////////////////
    // 미션 로직
    // - UI에서 선택한 MissionDataAsset을 GameInstnace에서 보관
    // - GameMode에서 이 데이터를 읽고 미션 시작
    ////////////////////////////////////////////////////////////////////////
    
    // 선택한 미션 DataAsset 저장
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetSelectedMission(ULA_MissionDataAsset* InMissionData);

    // 선택한 미션 DataAsset 리턴
    UFUNCTION(BlueprintPure, Category = "Mission")
    ULA_MissionDataAsset* GetSelectedMission() const;

    // 현재 선택된 미션의 PrimaryAssetId 리턴
    // SaveGAme에서 ID를 통해 아이템을 가져옴
    UFUNCTION(BlueprintPure, Category = "Mission")
    FPrimaryAssetId GetSelectedMissionId() const;


    ////////////////////////////////////////////////////////////////////////
    // 세이브 로직
    // - 체크 포인트: 게임 재개 / 게임 오버 후 재시작
    // - 미션 결과: 게임 결과 UI 출력
    ////////////////////////////////////////////////////////////////////////
    
    // 체크 포인트 상호 작용 시 메모리에 저장
    // SaveGameData()에서 저장
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveCheckPointData(int32 PhaseIndex, FVector SaveLocation, FRotator SaveRotation, int32 ElapsedGameTime, int32 SaveScore);

    // 미션 클리어 시 결과 저장
    // 게임 오버는 UI로만 전달
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveMissionResultData(int32 ClearTime, int32 FinalScore, const FString& FinalRank);

    // CheckPointData를 SaveSlot에 저장
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveGameData();

    // SaveSlot에서 데이터를 불러옴
    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadGameData();

    // 세이브 파일이 실제로 디스크에 존재하는지 여부를 리턴하는 함수
    UFUNCTION(BlueprintPure, Category = "Save")
    bool DoesSaveGameSlotExist() const;

    // 메인 메뉴에서 Continue시 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Save")
    void ContinueGame();

private:
    // 기존 SaveGame이 있으면 불러오고, 없으면 새로 생성
    ULA_SaveGame* LoadOrCreateSaveGameObject() const;

public:
    // 현재 미션에서 획득한 점수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 TotalScore;

    // 선택된 미션 Data Asset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    ULA_MissionDataAsset* SelectedMissionDataAsset;

    // 현재 체크포인트 데이터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FLA_CheckPointSaveData CheckPointData;

    // 기본 슬롯 이름
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save Data")
    FString SaveSlotName;

    // 마지막 저장 성공 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
    bool bSaveSuccess;
};
