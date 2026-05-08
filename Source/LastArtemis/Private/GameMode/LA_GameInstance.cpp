#include "GameMode/LA_GameInstance.h"

ULA_GameInstance::ULA_GameInstance()
    :
    TotalGold(0),
    TotalScore(0),
    SelectedMissionDataAsset(nullptr),

    SavedPhaseIndex(-1),
    SavedPlayerLocation(FVector::ZeroVector),
    SavedPlayerRotation(FRotator::ZeroRotator)
{
    SaveSlotName = TEXT("DefaultSaveSlot");
}

////////////////////////////////////////////////////////////////////////
/// 보상 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::AddReward(int32 GoldReward, int32 ScoreReward)
{
	TotalGold += GoldReward;
	TotalScore += ScoreReward;
}


void ULA_GameInstance::ResetPlayerData()
{
	TotalGold = 0;
    TotalScore = 0;
}

////////////////////////////////////////////////////////////////////////
/// 미션 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::SetSelectedMission(ULA_MissionDataAsset* InMissionData)
{
    SelectedMissionDataAsset = InMissionData;
}

ULA_MissionDataAsset* ULA_GameInstance::GetSelectedMission() const
{
    return SelectedMissionDataAsset;
}

void ULA_GameInstance::SaveCheckPointLocation(FVector SaveLocation, FRotator SaveRotation)
{
    SavedPlayerLocation = SaveLocation;
    SavedPlayerRotation = SaveRotation;
}

void ULA_GameInstance::SaveGameData()
{
}

void ULA_GameInstance::LoadGameData()
{
}


