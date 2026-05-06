// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_GameModeBase.h"
#include "LastArtemis\Character\LA_PlayerCharacter.h"
#include "LA_DefaultPlayerController.h"
#include "LA_GameStateBase.h"
#include "LA_GameType.h"
#include "Kismet/GameplayStatics.h"

ALA_GameModeBase::ALA_GameModeBase()
    :
    bCurrentPhaseCompleted(false)
{
	DefaultPawnClass = ALA_PlayerCharacter::StaticClass();
	PlayerControllerClass = ALA_DefaultPlayerController::StaticClass();
	GameStateClass = ALA_GameStateBase::StaticClass();
}

void ALA_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::MainMenu);
        LA_GameState->SetMissionType(ELA_MissionType::None);
	}
}

void ALA_GameModeBase::StartNewGame()
{
	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::MissionSelect);
        LA_GameState->SetMissionType(ELA_MissionType::None);
	}

    bCurrentPhaseCompleted = false;

    UGameplayStatics::SetGamePaused(GetWorld(), false);

}

void ALA_GameModeBase::LoadSaveGame()
{

}

void ALA_GameModeBase::SelectMission()
{
    if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
    {
        LA_GameState->SetMissionType(ELA_MissionType::Invasion);
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Playing);
    }

    bCurrentPhaseCompleted = false;

    StartPhase(0);
}

void ALA_GameModeBase::StartPhase(int32 PhaseIndex)
{
    if (!InvasionPhaseList.IsValidIndex(PhaseIndex))
    {
        OnGameClear();
        return;
    }

    const FLA_PhaseData& PhaseData = InvasionPhaseList[PhaseIndex];

    bCurrentPhaseCompleted = false;

    if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
    {
        LA_GameState->SetCurrentPhase(PhaseIndex, PhaseData);
    }

    switch (PhaseData.PhaseType)
    {
        case ELA_PhaseType::Combat:
            StartCombatPhase(PhaseData);
            break;
        case ELA_PhaseType::Rest:
            StartRestPhase(PhaseData);
            break;
        case ELA_PhaseType::Boss:
            StartBossPhase(PhaseData);
            break;
        default:
            break;
    }
}

void ALA_GameModeBase::StartCombatPhase(const FLA_PhaseData& PhaseData)
{
    // enemy 관련 로직
}

void ALA_GameModeBase::StartRestPhase(const FLA_PhaseData& PhaseData)
{
    // 휴식 관련 로직
    CompleteCurrentPhase();
}

void ALA_GameModeBase::StartBossPhase(const FLA_PhaseData& PhaseData)
{
    // 보스 관련 로직
}

void ALA_GameModeBase::CompleteCurrentPhase()
{
    if (bCurrentPhaseCompleted)
    {
        return;
    }

    bCurrentPhaseCompleted = true;
}

void ALA_GameModeBase::AdvanceToNextPhase()
{
    // 현재 Phase 클리어 못함
    if (!bCurrentPhaseCompleted)
        return;

    // 현재 Phase 클리어
    if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
    {
        // 다음 Phase로 이동 가능
        const int32 NextPhaseIndex = LA_GameState->GetCurrentPhaseIndex() + 1;

        bCurrentPhaseCompleted = false;

        // 다음 Phase Index 없을 시 게임 클리어
        if (!InvasionPhaseList.IsValidIndex(NextPhaseIndex))
        {
            OnGameClear();
            return;
        }

        StartPhase(NextPhaseIndex);
    }
}


void ALA_GameModeBase::PauseGame()
{
	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Paused);
	}

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ALA_GameModeBase::ResumeGame()
{
	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Playing);
	}

    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void ALA_GameModeBase::OnGameOver()
{
	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameOver);
	}

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ALA_GameModeBase::OnGameClear()
{
	if (ALA_GameStateBase* LA_GameState = GetGameState<ALA_GameStateBase>())
	{
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameClear);
	}
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}



