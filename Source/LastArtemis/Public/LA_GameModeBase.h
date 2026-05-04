// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LA_GameModeBase.generated.h"


UCLASS()
class LASTARTEMIS_API ALA_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALA_GameModeBase();

protected:
	virtual void BeginPlay() override;

	// 메인 메뉴
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartNewGame();		// 새 게임 시작
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void LoadSaveGame();		// 저장된 게임 불러오기

	// Wave 시작 / 종료
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 WaveIndex);	// WaveIndex 웨이브 시작
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void EndWave();						// 웨이브 종료

	// 일시 정지
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void PauseGame();		// 게임 일시 정지
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ResumeGame();		// 게임 재개

	// 게임 종료 / 클리어
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameOver();		// 게임 종료
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameClear();		// 게임 클리어
};
