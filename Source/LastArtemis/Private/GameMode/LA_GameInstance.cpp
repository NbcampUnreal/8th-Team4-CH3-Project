#include "GameMode/LA_GameInstance.h"
#include "GameMode/LA_SaveGame.h"
#include "GameMode/LA_GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ULA_GameInstance::ULA_GameInstance()
    :
    TotalGold(0),
    TotalScore(0),
    SelectedMissionDataAsset(nullptr),

    SavedPhaseIndex(-1),
    SavedPlayerLocation(FVector::ZeroVector),
    SavedPlayerRotation(FRotator::ZeroRotator),
    bSaveSuccess(false)
{
    SaveSlotName = TEXT("SaveSlot");
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

void ULA_GameInstance::SaveCheckPointData(int32 PhaseIndex, FVector SaveLocation, FRotator SaveRotation)
{
    SavedPhaseIndex = PhaseIndex;
    SavedPlayerLocation = SaveLocation;
    SavedPlayerRotation = SaveRotation;
}

////////////////////////////////////////////////////////////////////////
/// 세이브 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::SaveGameData()
{
    ULA_SaveGame* LA_SaveGame = Cast<ULA_SaveGame>(
        UGameplayStatics::CreateSaveGameObject(ULA_SaveGame::StaticClass())
    );

    if (!LA_SaveGame)
        return;

    LA_SaveGame->SavedMissionDataAsset = SelectedMissionDataAsset;
    LA_SaveGame->SavedPhaseIndex = SavedPhaseIndex;
    LA_SaveGame->SavedPlayerLocation = SavedPlayerLocation;
    LA_SaveGame->SavedPlayerRotation = SavedPlayerRotation;
    LA_SaveGame->SavedGold = TotalGold;
    LA_SaveGame->SavedScore = TotalScore;

    UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    bSaveSuccess = UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    UE_LOG(LogTemp, Warning, TEXT("Save Data - Success: %s, PhaseIndex: %d, Location: %s"),
        bSaveSuccess ? TEXT("True") : TEXT("False"),
        SavedPhaseIndex,
        *SavedPlayerLocation.ToString()
    );
}

void ULA_GameInstance::LoadGameData()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
        return;

    ULA_SaveGame* LA_SaveGame = Cast<ULA_SaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
    );

    if (!LA_SaveGame)
        return;

    SelectedMissionDataAsset = LA_SaveGame->SavedMissionDataAsset;

    SavedPhaseIndex = LA_SaveGame->SavedPhaseIndex;
    SavedPlayerLocation = LA_SaveGame->SavedPlayerLocation;
    SavedPlayerRotation = LA_SaveGame->SavedPlayerRotation;

    TotalGold = LA_SaveGame->SavedGold;
    TotalScore = LA_SaveGame->SavedScore;

    bSaveSuccess = UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    UE_LOG(LogTemp, Warning, TEXT("Load Data - Success: %s, PhaseIndex: %d, Location: %s"),
        bSaveSuccess ? TEXT("True") : TEXT("False"),
        SavedPhaseIndex,
        *SavedPlayerLocation.ToString()
    );
}


