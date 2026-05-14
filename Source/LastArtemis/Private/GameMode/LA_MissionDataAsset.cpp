// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_MissionDataAsset.h"

void ULA_MissionDataAsset::GetPhaseInfo_Implementation(int32 PhaseIndex, FText& OutObjectiveText, int32& OutRequiredCount) const
{
    OutObjectiveText = FText::FromString(TEXT("Base Mission Data"));
    OutRequiredCount = 0;
}

FPrimaryAssetId ULA_MissionDataAsset::GetPrimaryAssetId() const
{
    const FName DataAssetName = MissionId.IsNone() ? GetFName() : MissionId;

    return FPrimaryAssetId(TEXT("Mission"), DataAssetName);
}
