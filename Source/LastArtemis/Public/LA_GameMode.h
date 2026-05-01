// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LA_GameInstance.h"
#include "LA_GameMode.generated.h"


UCLASS()
class LASTARTEMIS_API ALA_GameMode : public AGameMode
{
	GENERATED_BODY()

public:
    ALA_GameMode();

	// 게임 시작 
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGame();		// 게임 시작

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameOver();		// 게임 종료

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnGameClear();		// 게임 클리어
	
	

};
