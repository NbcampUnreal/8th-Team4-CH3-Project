// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LA_GameType.generated.h"

// Game Flow State
UENUM(BlueprintType)
enum class ELA_GameFlowState : uint8
{
	None,
	MainMenu,
	Playing,
	Paused,
	GameOver,
	GameClear,
};