// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LA_SaveGame.generated.h"

/**
 * 
 */
UCLASS()
class LASTARTEMIS_API ULA_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    // 보상
    UPROPERTY()
    int32 SavedGold = 0;
    UPROPERTY()
    int32 SavedScore = 0;

    // 지금까지 도달한 Rest Phase
    UPROPERTY()
    int32 SavedPhaseIndex = -1;

    // 세이브 위치
    UPROPERTY()
    FVector SavedPlayerLocation = FVector::ZeroVector;
    UPROPERTY()
    FRotator SavedPlayerRotation = FRotator::ZeroRotator;
};
