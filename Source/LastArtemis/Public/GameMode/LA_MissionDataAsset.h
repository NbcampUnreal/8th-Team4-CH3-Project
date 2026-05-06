// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/LA_GameType.h"
#include "LA_MissionDataAsset.generated.h"

class ULA_MissionLogic;

UCLASS()
class LASTARTEMIS_API ULA_MissionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    FText MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    ELA_MissionType MissionType = ELA_MissionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    TSubclassOf<ULA_MissionLogic> MissionLogicClass;
};
