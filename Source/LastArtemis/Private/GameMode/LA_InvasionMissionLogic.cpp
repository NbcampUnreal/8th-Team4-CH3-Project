// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_InvasionMissionLogic.h"
#include "GameMode/LA_InvasionMissionDataAsset.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_GameModeBase.h"
#include "UI/LA_GameType.h"

void ULA_InvasionMissionLogic::StartPhase(int32 PhaseIndex)
{
    ULA_InvasionMissionDataAsset* InvasionDataAsset = Cast<ULA_InvasionMissionDataAsset>(MissionDataAsset);
    if (!InvasionDataAsset)
        return;

    // Data Asset의 범위를 넘어가면 게임 클리어
    if (!InvasionDataAsset->PhaseList.IsValidIndex(PhaseIndex))
    {
        if (GameMode)
        {
            GameMode->OnGameClear();
            return;
        }
    }

    const FLA_InvasionPhaseData& PhaseData = InvasionDataAsset->PhaseList[PhaseIndex];

    // Game State 정보 업데이트
    if (GameState)
    {
        GameState->SetMissionType(ELA_MissionType::Invasion);
        GameState->SetCurrentPhaseInfo(
            PhaseIndex,
            PhaseData.PhaseType,
            PhaseData.ObjectiveText,
            PhaseData.RequiredProgressCount
        );
    }

    // Phase Type에 따른 미션 실행
    HandleInvasionObjective(PhaseData);
}

void ULA_InvasionMissionLogic::AdvanceToNextPhase()
{
    if (!GameState)
        return;

    if (!GameState->IsCurrentPhaseCompleted())
        return;

    const int32 NextPhaseIndex = GameState->GetCurrentPhaseIndex() + 1;
    StartPhase(NextPhaseIndex);
}

void ULA_InvasionMissionLogic::HandleInvasionObjective(const FLA_InvasionPhaseData& PhaseData)
{
    switch (PhaseData.ObjectiveType)
    {
    case ELA_InvasionObjectiveType::None:
        CompleteCurrentPhase();
        break;

    case ELA_InvasionObjectiveType::DisableTurret:
        // 자동 포탑 무력화 로직 구현
        break;

    case ELA_InvasionObjectiveType::ControlPassage:
        // 핵심 연결 통로 제어 로직 구현
        break;

    case ELA_InvasionObjectiveType::BreakDefenseSystem:
        // 방어 시스템 돌파 로직 구현
        break;

    case ELA_InvasionObjectiveType::KillBoss:
        // 보스 처치 목표 로직 구현
        break;

    default:
        break;
    }
}
