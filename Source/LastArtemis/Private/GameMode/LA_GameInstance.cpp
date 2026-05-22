#include "GameMode/LA_GameInstance.h"
#include "GameMode/LA_SaveGame.h"
#include "GameMode/LA_SaveGameSettings.h"
#include "GameMode/LA_MissionDataAsset.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

ULA_GameInstance::ULA_GameInstance()
    :
    TotalScore(0),
    SelectedMissionDataAsset(nullptr),
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

void ULA_GameInstance::UpdateAndSaveVolume(float NewBGMVolume, float NewSFXVolume)
{

    // 저장할 볼륨 설정
    CurrentBGMVolume = NewBGMVolume;
    CurrentSFXVolume = NewSFXVolume;

    // 즉시 파일로 저장
    ULA_SaveGameSettings* SaveObj = Cast<ULA_SaveGameSettings>(UGameplayStatics::CreateSaveGameObject(ULA_SaveGameSettings::StaticClass()));
    if (SaveObj)
    {
        SaveObj->SavedBGMVolume = CurrentBGMVolume;
        SaveObj->SavedSFXVolume = CurrentSFXVolume;
        SaveObj->SavedAimInputMode = CurrentAimInputMode;
        SaveObj->SavedSprintInputMode = CurrentSprintInputMode;

        UGameplayStatics::SaveGameToSlot(SaveObj, SettingsSlotName, 0);
    }
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
            CurrentBGMVolume = LoadObj->SavedBGMVolume;
            CurrentSFXVolume = LoadObj->SavedSFXVolume;
        }
        else
        {
        CurrentAimInputMode = EMovementInputMode::Toggle;
        CurrentSprintInputMode = EMovementInputMode::Toggle;
        CurrentBGMVolume = 0.5f;
        CurrentSFXVolume = 0.5f;
        }
    }
}

////////////////////////////////////////////////////////////////////////
/// 점수 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::AddScore(int32 ScoreAmount)
{
    if (ScoreAmount <= 0)
        return;

	TotalScore += ScoreAmount;
}


void ULA_GameInstance::ResetScore()
{
    TotalScore = 0;
}

int32 ULA_GameInstance::GetScore()
{
    return TotalScore;
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

FPrimaryAssetId ULA_GameInstance::GetSelectedMissionId() const
{
    if (!SelectedMissionDataAsset)
    {
        return FPrimaryAssetId();
    }

    return SelectedMissionDataAsset->GetPrimaryAssetId();
}

////////////////////////////////////////////////////////////////////////
/// 세이브 로직
////////////////////////////////////////////////////////////////////////

ULA_SaveGame* ULA_GameInstance::LoadOrCreateSaveGameObject() const
{
    ULA_SaveGame* LA_SaveGame = nullptr;

    // Save File이 있으면 불러오기
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        LA_SaveGame = Cast<ULA_SaveGame>(
            UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
        );
    }

    // Save File이 없으면 새로 생성
    if (!LA_SaveGame)
    {
        LA_SaveGame = Cast<ULA_SaveGame>(
            UGameplayStatics::CreateSaveGameObject(ULA_SaveGame::StaticClass())
        );
    }

    return LA_SaveGame;
}


void ULA_GameInstance::SaveCheckPointData(int32 PhaseIndex, FVector SaveLocation, FRotator SaveRotation, int32 ElapsedGameTime, int32 SaveScore)
{
    const FPrimaryAssetId CurrentMissionId = GetSelectedMissionId();
    if (!CurrentMissionId.IsValid())
        return;

    CheckPointData.MissionId = CurrentMissionId;
    CheckPointData.PhaseIndex = PhaseIndex;
    CheckPointData.PlayerLocation = SaveLocation;
    CheckPointData.PlayerRotation = SaveRotation;
    CheckPointData.ElapsedGameTime = ElapsedGameTime;
    CheckPointData.SavedScore = SaveScore;
}

void ULA_GameInstance::SaveMissionResultData(int32 ClearTime, int32 FinalScore, const FString& FinalRank)
{
    const FPrimaryAssetId CurrentMissionId = GetSelectedMissionId();
    if (!CurrentMissionId.IsValid())
    {
        bSaveSuccess = false;
        return;
    }

    ULA_SaveGame* LA_SaveGame = LoadOrCreateSaveGameObject();
    if (!LA_SaveGame)
        return;

    // 기존 미션 결과 데이터 가져오기
    FLA_MissionResultSaveData* ResultData = LA_SaveGame->MissionResultData.FindByPredicate(
        [&CurrentMissionId]
        (const FLA_MissionResultSaveData& Data)
        {
            return Data.MissionId == CurrentMissionId;
        }
    );

    // 기존 미션 결과 데이터가 SaveGame에 없으면 새로 생성
    if (!ResultData)
    {
        FLA_MissionResultSaveData NewResultData;
        NewResultData.MissionId = CurrentMissionId;

        LA_SaveGame->MissionResultData.Add(NewResultData);
        ResultData = &LA_SaveGame->MissionResultData.Last();
    }

    // 미션 클리어 여부
    // 미션 실패 시 저장 X
    ResultData->bCleared = true;

    // 베스트 클리어 시간 갱신
    if (ResultData->BestClearTime <= 0 || ClearTime < ResultData->BestClearTime)
    {
        ResultData->BestClearTime = ClearTime;
    }

    // 최고 점수 갱신
    if (FinalScore > ResultData->BestScore)
    {
        ResultData->BestScore = FinalScore;
        ResultData->BestRank = FinalRank;
    }

    if (ResultData->BestRank.IsEmpty())
    {
        ResultData->BestRank = FinalRank;
    }

    bSaveSuccess = UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    UE_LOG(LogTemp, Warning, TEXT("Mission Result Save - MissionId: %s, Time: %d, Score: %d, Rank: %s"),
        *CurrentMissionId.ToString(),
        ClearTime,
        FinalScore,
        *FinalRank
    );
}

// SaveSlot에 저장
void ULA_GameInstance::SaveGameData()
{
    ULA_SaveGame* LA_SaveGame = LoadOrCreateSaveGameObject();

    if (!LA_SaveGame)
    {
        bSaveSuccess = false;
        return;
    }

    if (!CheckPointData.IsValid())
    {
        bSaveSuccess = false;
        return;
    }

    LA_SaveGame->CheckPointData = CheckPointData;
    bSaveSuccess = UGameplayStatics::SaveGameToSlot(LA_SaveGame, SaveSlotName, 0);

    UE_LOG(LogTemp, Warning, TEXT("CheckPoint Save - MissionId: %s, PhaseIndex: %d, Location: %s, ElapsedTime: %d"),
        *CheckPointData.MissionId.ToString(),
        CheckPointData.PhaseIndex,
        *CheckPointData.PlayerLocation.ToString(),
        CheckPointData.ElapsedGameTime
    );
}

// SaveSLot에서 체크포인트 데이터 불러와 GameInstance에 복구
void ULA_GameInstance::LoadGameData()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
        return;

    ULA_SaveGame* LA_SaveGame = Cast<ULA_SaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
    );

    if (!LA_SaveGame)
        return;

    CheckPointData = LA_SaveGame->CheckPointData;
    TotalScore = CheckPointData.SavedScore;

    // 미션 ID까지 복구
    if (CheckPointData.MissionId.IsValid())
    {
        UObject* MissionObject = UAssetManager::Get().GetPrimaryAssetObject(CheckPointData.MissionId);

        if (!MissionObject)
        {
            const FSoftObjectPath MissionPath = UAssetManager::Get().GetPrimaryAssetPath(CheckPointData.MissionId);
            MissionObject = MissionPath.TryLoad();
        }

        SelectedMissionDataAsset = Cast<ULA_MissionDataAsset>(MissionObject);
    }

    bSaveSuccess = true;

    UE_LOG(LogTemp, Warning, TEXT("Load Data - MissionId: %s, PhaseIndex: %d, Location: %s, ElapsedTime: %d"),
        *CheckPointData.MissionId.ToString(),
        CheckPointData.PhaseIndex,
        *CheckPointData.PlayerLocation.ToString(),
        CheckPointData.ElapsedGameTime
    );
}

bool ULA_GameInstance::DoesSaveGameSlotExist() const
{
    return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

void ULA_GameInstance::ContinueGame()
{
    // 1. 디스크에서 세이브 파일을 읽어 CheckPointData 변수를 채웁니다.
    LoadGameData();

    // 2. 불러온 체크포인트 데이터의 MissionId가 유효한지 검증합니다.
    if (bSaveSuccess && CheckPointData.IsValid())
    {
        // 3. AssetManager를 통해 PrimaryAssetId로부터 MissionDataAsset 포인터를 찾아옵니다.
        UAssetManager& AssetManager = UAssetManager::Get();
        ULA_MissionDataAsset* LoadedMission = Cast<ULA_MissionDataAsset>(
            AssetManager.GetPrimaryAssetObject(CheckPointData.MissionId)
        );

        // 만약 메모리에 로드되어 있지 않다면 동기식 강제 로드 시도
        if (!LoadedMission)
        {
            FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(CheckPointData.MissionId);
            LoadedMission = Cast<ULA_MissionDataAsset>(AssetPath.TryLoad());
        }

        if (LoadedMission)
        {
            // 4. 기존 변수 SelectedMissionDataAsset에 로드된 미션을 지정합니다.
            SetSelectedMission(LoadedMission);

            // 5. MissionDataAsset에 들어있는 타겟 맵(레벨)을 엽니다.
            // ※ 데이터 에셋 내부에 맵 이름을 가지는 FName 변수(예: MapName)가 있다고 전제합니다.
            FName TargetMapName = SelectedMissionDataAsset->LevelName; 
            
            if (!TargetMapName.IsNone())
            {
                UGameplayStatics::OpenLevel(this, TargetMapName);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Continue Error: MissionDataAsset에 MapName이 지정되지 않았습니다."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Continue 취소: 유효한 세이브 파일이 슬롯에 없습니다."));
    }
}



