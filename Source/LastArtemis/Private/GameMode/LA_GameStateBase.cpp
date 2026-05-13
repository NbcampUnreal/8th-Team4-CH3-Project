// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_GameModeBase.h"

ALA_GameStateBase::ALA_GameStateBase()
    :
    CurrentGameFlowState(ELA_GameFlowState::None),
    CurrentMissionType(ELA_MissionType::None),
    CurrentPhaseIndex(-1),
    CurrentPhaseType(ELA_PhaseType::None),
    CurrentProgressCount(0),
    RequiredProgressCount(0),
    bCurrentPhaseCompleted(false),
    ElapsedGameTime(0)
{
}

////////////////////////
// 게임 흐름 / 미션 정보 
////////////////////////

void ALA_GameStateBase::SetGameFlowState(ELA_GameFlowState NewState)
{
    CurrentGameFlowState = NewState;
}

ELA_GameFlowState ALA_GameStateBase::GetGameFlowState() const
{
    return CurrentGameFlowState;
}

void ALA_GameStateBase::SetMissionType(ELA_MissionType NewMissionType)
{
    CurrentMissionType = NewMissionType;
}

ELA_MissionType ALA_GameStateBase::GetMissionType() const
{
    return CurrentMissionType;
}

////////////////////////
// Phase / Objective 
////////////////////////

void ALA_GameStateBase::SetCurrentPhaseInfo(
    int32 NewPhaseIndex,
    ELA_PhaseType NewPhaseType,
    const FText& NewObjectiveText,
    int32 NewRequiredProgressCount
)
{
    // GameLogic에서 전달 받은 정보 설정
    CurrentPhaseIndex = NewPhaseIndex;
    CurrentPhaseType = NewPhaseType;
    CurrentObjectiveText = NewObjectiveText;

    // Phase 갱신 시, 진행도 초기화
    CurrentProgressCount = 0;
    RequiredProgressCount = NewRequiredProgressCount;
    bCurrentPhaseCompleted = false;

    if (ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(GetWorld()->GetAuthGameMode()))
    {
        OnMissionStatusChanged.Broadcast(GM->GetMissionDataAsset(), CurrentPhaseIndex, CurrentProgressCount);
    }
}

void ALA_GameStateBase::AddObjectiveProgress(int32 AddCount)
{
    if (AddCount <= 0)
        return;

    CurrentProgressCount += AddCount;

    if (ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(GetWorld()->GetAuthGameMode()))
    {
        OnMissionStatusChanged.Broadcast(GM->GetMissionDataAsset(), CurrentPhaseIndex, CurrentProgressCount);
    }
}

void ALA_GameStateBase::ResetObjectiveProgress()
{
    CurrentProgressCount = 0;
    if (ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(GetWorld()->GetAuthGameMode()))
    {
        OnMissionStatusChanged.Broadcast(GM->GetMissionDataAsset(), CurrentPhaseIndex, CurrentProgressCount);
    }
}

void ALA_GameStateBase::SetCurrentPhaseCompleted(bool bCompleted)
{
    bCurrentPhaseCompleted = bCompleted;
}

bool ALA_GameStateBase::IsCurrentPhaseCompleted() const
{
    return bCurrentPhaseCompleted;
}

bool ALA_GameStateBase::IsObjectiveCompleted() const
{
    // 목표 Objective 진행도가 0인 경우 true로 반환
    // 그 외는 현재 진행도 >= 목표 진행도인 경우에만 true
    return RequiredProgressCount <= 0 || CurrentProgressCount >= RequiredProgressCount;
}

int32 ALA_GameStateBase::GetCurrentPhaseIndex() const
{
    return CurrentPhaseIndex;
}

ELA_PhaseType ALA_GameStateBase::GetCurrentPhaseType() const
{
    return CurrentPhaseType;
}

int32 ALA_GameStateBase::GetCurrentProgressCount() const
{
    return CurrentProgressCount;
}

int32 ALA_GameStateBase::GetRequiredProgressCount() const
{
    return RequiredProgressCount;
}

FText ALA_GameStateBase::GetCurrentObjectiveText() const
{
    return CurrentObjectiveText;
}

////////////////////////
// 타이머
////////////////////////

void ALA_GameStateBase::ResetElapsedGameTime()
{
    ElapsedGameTime = 0;
}

void ALA_GameStateBase::AddElapsedGameTime(int32 AddSeconds)
{
    if (AddSeconds <= 0)
        return;

    ElapsedGameTime += AddSeconds;
}

int32 ALA_GameStateBase::GetElapsedGameTime() const
{
    return ElapsedGameTime;
}

FText ALA_GameStateBase::GetElapsedGameTimeText() const
{
    const int32 Minutes = ElapsedGameTime / 60;
    const int32 Seconds = ElapsedGameTime % 60;

    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}
