// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/LA_MissionDataAsset.h"
#include "UI/LA_GameType.h"
#include "LA_InvasionMissionDataAsset.generated.h"


UCLASS()
class LASTARTEMIS_API ULA_InvasionMissionDataAsset : public ULA_MissionDataAsset
{
	GENERATED_BODY()

public:
    ULA_InvasionMissionDataAsset()
    {
        MissionType = ELA_MissionType::Invasion;
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Invasion")
    TArray<FLA_InvasionPhaseData> PhaseList;

};
