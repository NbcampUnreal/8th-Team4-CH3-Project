// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/LA_GameType.h"
#include "LA_GameModeBase.generated.h"

class ALA_GameStateBase;
class ULA_MissionDataAsset;
class ULA_MissionLogic;

UCLASS(Blueprintable)
class LASTARTEMIS_API ALA_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALA_GameModeBase();

protected:
    virtual void BeginPlay() override;

public:
    ALA_GameStateBase* GetLAGameState() const;

    ////////////////////////
    /// 게음 흐름 제어
    ////////////////////////
    // 새 게임 시작
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartNewGame();

    // 게임 불러오기
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void LoadSavedGame();

    // 일시 정지 (UI로 조작 가능)
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void PauseGame();

    // 일시정지 된 게임 재개
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void ResumeGame();

    // 게임 오버 조건 충족 시 호출
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void OnGameOver();

    // 게임 클리어 시 호출
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void OnGameClear();


    ////////////////////////
    /// 미션 진행 제어
    ////////////////////////

    // 위젯 클래스
    TSubclassOf<UUserWidget> HUDClass;

    // 현재 미션 데이터(MissionDataAsset)을 바탕으로 Phase 시작
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission();

    // 현재 Phase의 목표 진행도 증가
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void AddObjectiveProgress(int32 AddCount);
    // 현재 Phase의 목표 완료
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteCurrentPhase();
    // 다음 Phase 진행
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    ULA_MissionDataAsset* GetMissionDataAsset();

protected:
    // Mission Data Asset 할당
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
    TObjectPtr<ULA_MissionDataAsset> MissionDataAsset;

    // 게임 시작 시 할당된 MissionDataAsset을 기반으로 Mission Logic 진행
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
    TObjectPtr<ULA_MissionLogic> CurrentMissionLogic;
};
