// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LA_GameType.h"
#include "LA_GameStateBase.h"
#include "LA_GameModeBase.generated.h"


UCLASS()
class LASTARTEMIS_API ALA_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALA_GameModeBase();

protected:
    virtual void BeginPlay() override;

public:
	// 메인 메뉴
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void StartNewGame();		// 새 게임 시작
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void LoadSaveGame();		// 저장된 게임 불러오기

    // 미션 선택
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SelectMission();

    // Phase 진행
    UFUNCTION(BlueprintCallable, Category = "Phase")
    void CompleteCurrentPhase();

    UFUNCTION(BlueprintCallable, Category = "Phase")
    void AdvanceToNextPhase();

	// 일시 정지
	UFUNCTION(BlueprintCallable, Category = "Paused")
	void PauseGame();		// 게임 일시 정지
	UFUNCTION(BlueprintCallable, Category = "Paused")
	void ResumeGame();		// 게임 재개

	// 게임 종료 / 클리어
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameOver();		// 게임 종료
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameClear();		// 게임 클리어

protected:
    void StartPhase(int32 PhaseIndex);

    void StartCombatPhase(const FLA_PhaseData& PhaseData);
    void StartRestPhase(const FLA_PhaseData& PhaseData);
    void StartBossPhase(const FLA_PhaseData& PhaseData);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Invasion")
    TArray<FLA_PhaseData> InvasionPhaseList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
    bool bCurrentPhaseCompleted = false;
};
