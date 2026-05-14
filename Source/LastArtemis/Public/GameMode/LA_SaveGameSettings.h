// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "LA_SaveGameSettings.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_SaveGameSettings : public USaveGame
{
	GENERATED_BODY()

public:
    
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EMovementInputMode SavedAimInputMode = EMovementInputMode::Toggle;

    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    EMovementInputMode SavedSprintInputMode = EMovementInputMode::Toggle;

    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    float MasterVolume = 1.0f;
};
