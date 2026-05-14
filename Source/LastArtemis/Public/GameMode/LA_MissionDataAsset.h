// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/LA_GameType.h"
#include "LA_MissionDataAsset.generated.h"

class ULA_MissionLogic;

UCLASS(BlueprintType, Blueprintable)
class LASTARTEMIS_API ULA_MissionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    FName MissionId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    FText MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta = (MultiLine = true))
    FText MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    FName LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    ELA_MissionType MissionType = ELA_MissionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    TSubclassOf<ULA_MissionLogic> MissionLogicClass;

public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Mission")
    void GetPhaseInfo(int32 PhaseIndex, FText& OutDescription, int32& OutMaxCount) const;
};
