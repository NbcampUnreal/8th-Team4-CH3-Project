// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_InvasionMissionDataAsset.h"

void ULA_InvasionMissionDataAsset::GetPhaseInfo_Implementation(int32 PhaseIndex, FText& ObjectiveText, int32& RequiredCount) const
{
    // 리스트 범위를 벗어나지 않는지 확인
    if (PhaseList.IsValidIndex(PhaseIndex))
    {
        ObjectiveText = PhaseList[PhaseIndex].ObjectiveText;
        RequiredCount = PhaseList[PhaseIndex].RequiredProgressCount;
    }
    else
    {
        ObjectiveText = FText::FromString(TEXT("Mission Information Missing"));
        RequiredCount = 0;
    }
}
