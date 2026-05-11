// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LA_SaveGame.generated.h"

class ULA_MissionDataAsset;

UCLASS(BlueprintType)
class LASTARTEMIS_API ULA_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    ULA_SaveGame();

public:
    // 미션
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    ULA_MissionDataAsset* SavedMissionDataAsset;


    // 지금까지 도달한 Rest Phase
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    int32 SavedPhaseIndex;

    // 보상
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    int32 SavedGold;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    int32 SavedScore;

    // 세이브 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FVector SavedPlayerLocation;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FRotator SavedPlayerRotation;
};
