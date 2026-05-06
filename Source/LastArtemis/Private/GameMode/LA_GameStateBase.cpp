// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_GameStateBase.h"

ALA_GameStateBase::ALA_GameStateBase()
	:
	CurrentGameFlowState(ELA_GameFlowState::None),
    CurrentMissionType(ELA_MissionType::None),
    CurrentPhaseIndex(-1),
    CurrentPhaseType(ELA_PhaseType::None),
    CurrentKillCount(0),
    RequiredKillCount(0)
{

}

//////////////////////////////
// Game Flow State
//////////////////////////////

void ALA_GameStateBase::SetGameFlowState(ELA_GameFlowState NewState)
{
	CurrentGameFlowState = NewState;
}

ELA_GameFlowState ALA_GameStateBase::GetGameFlowState() const
{
	return CurrentGameFlowState;
}

//////////////////////////////
// Mission
//////////////////////////////

void ALA_GameStateBase::SetMissionType(ELA_MissionType NewMissionType)
{
    CurrentMissionType = NewMissionType;
}

ELA_MissionType ALA_GameStateBase::GetMissionType() const
{
    return CurrentMissionType;
}

//////////////////////////////
// Phase
//////////////////////////////
void ALA_GameStateBase::SetCurrentPhase(int32 NewPhaseIndex, const FLA_PhaseData& PhaseData)
{
    CurrentPhaseIndex = NewPhaseIndex;
    CurrentPhaseType = PhaseData.PhaseType;
    CurrentObjectiveText = PhaseData.ObjectiveText;

    CurrentKillCount = 0;
    RequiredKillCount = PhaseData.RequiredKillCount;
}

void ALA_GameStateBase::AddKillCount(int32 AddCount)
{
    CurrentKillCount += AddCount;
}

void ALA_GameStateBase::ResetKillCount()
{
    CurrentKillCount = 0;
}

bool ALA_GameStateBase::IsKillObjectiveCompleted() const
{
    return CurrentKillCount >= RequiredKillCount;
}

int32 ALA_GameStateBase::GetCurrentPhaseIndex() const
{
    return CurrentPhaseIndex;
}

ELA_PhaseType ALA_GameStateBase::GetCurrentPhaseType() const
{
    return CurrentPhaseType;
}

int32 ALA_GameStateBase::GetCurrentKillCount() const
{
    return CurrentKillCount;
}

int32 ALA_GameStateBase::GetRequiredKillCount() const
{
    return RequiredKillCount;
}
