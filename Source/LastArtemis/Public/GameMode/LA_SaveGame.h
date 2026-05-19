// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/PrimaryAssetId.h"
#include "LA_SaveGame.generated.h"

class ULA_MissionDataAsset;

USTRUCT(BlueprintType)
struct FLA_CheckPointSaveData
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    FPrimaryAssetId MissionId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    int32 PhaseIndex = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    FVector PlayerLocation = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    int32 ElapsedGameTime = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    int32 SavedScore = 0;

    bool IsValid() const
    {
        return MissionId.IsValid() && PhaseIndex >= 0;
    }
};

USTRUCT(BlueprintType)
struct FLA_MissionResultSaveData
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Result")
    FPrimaryAssetId MissionId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Result")
    bool bCleared = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Result")
    int32 BestClearTime = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Result")
    int32 BestScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Result")
    FString BestRank;
};


UCLASS(BlueprintType)
class LASTARTEMIS_API ULA_SaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    ULA_SaveGame();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    FLA_CheckPointSaveData CheckPointData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Data")
    TArray<FLA_MissionResultSaveData> MissionResultData;
};
