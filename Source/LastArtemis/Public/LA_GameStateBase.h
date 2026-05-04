// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LA_GameType.h"
#include "LA_GameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ALA_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALA_GameStateBase();

	// game flow state
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameFlowState(ELA_GameFlowState NewState);
	UFUNCTION(BlueprintPure, Category = "Game State")
	ELA_GameFlowState GetGameFlowState() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
	ELA_GameFlowState CurrentGameFlowState;
};
