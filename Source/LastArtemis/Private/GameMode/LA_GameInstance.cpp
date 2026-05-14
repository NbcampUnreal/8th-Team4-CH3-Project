#include "GameMode/LA_GameInstance.h"
#include "GameMode/LA_SaveGame.h"
#include "GameMode/LA_SaveGameSettings.h"
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
/// 옵션 설정값 저장
///////////////////////////////////////////////////////////////////////

void ULA_GameInstance::Init()
{
    Super::Init();
    LoadSettingsFromDisk();
}

void ULA_GameInstance::UpdateAndSaveSettings(EMovementInputMode NewAimMode, EMovementInputMode NewSprintMode)
{
    // 메모리 값 업데이트
    CurrentAimInputMode = NewAimMode;
    CurrentSprintInputMode = NewSprintMode;

    // 즉시 파일로 저장
    ULA_SaveGameSettings* SaveObj = Cast<ULA_SaveGameSettings>(UGameplayStatics::CreateSaveGameObject(ULA_SaveGameSettings::StaticClass()));
    if (SaveObj)
    {
        SaveObj->SavedAimInputMode = CurrentAimInputMode;
        SaveObj->SavedSprintInputMode = CurrentSprintInputMode;
        UGameplayStatics::SaveGameToSlot(SaveObj, SettingsSlotName, 0);
    }

    // 캐릭터가 있다면 즉시 반영
    ApplySettingsToCharacter();
}

void ULA_GameInstance::ApplySettingsToCharacter()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        ALA_PlayerCharacter* Character = Cast<ALA_PlayerCharacter>(PC->GetPawn());
        if (Character)
        {
            Character->AimInputMode = CurrentAimInputMode;
            Character->SprintInputMode = CurrentSprintInputMode;
        }
    }
}

void ULA_GameInstance::LoadSettingsFromDisk()
{
    if (UGameplayStatics::DoesSaveGameExist(SettingsSlotName, 0))
    {
        ULA_SaveGameSettings* LoadObj = Cast<ULA_SaveGameSettings>(UGameplayStatics::LoadGameFromSlot(SettingsSlotName, 0));
        if (LoadObj)
        {
            CurrentAimInputMode = LoadObj->SavedAimInputMode;
            CurrentSprintInputMode = LoadObj->SavedSprintInputMode;
        }
    }
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

////////////////////////////////////////////////////////////////////////
/// 세이브 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::SaveCheckPointData(int32 PhaseIndex, FVector SaveLocation, FRotator SaveRotation, int32 ElapsedGameTime)
{
    SavedPhaseIndex = PhaseIndex;
    SavedPlayerLocation = SaveLocation;
    SavedPlayerRotation = SaveRotation;
    SavedElapsedGameTime = ElapsedGameTime;
}

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

    LA_SaveGame->SavedElapsedGameTime = SavedElapsedGameTime;

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

    SavedElapsedGameTime = LA_SaveGame->SavedElapsedGameTime;

    bSaveSuccess = UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    UE_LOG(LogTemp, Warning, TEXT("Load Data - Success: %s, PhaseIndex: %d, Location: %s"),
        bSaveSuccess ? TEXT("True") : TEXT("False"),
        SavedPhaseIndex,
        *SavedPlayerLocation.ToString()
    );
}


