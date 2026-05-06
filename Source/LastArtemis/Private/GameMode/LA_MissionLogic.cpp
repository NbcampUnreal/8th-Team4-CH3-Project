#include "GameMode/LA_MissionLogic.h"
#include "GameMode/LA_GameModeBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_MissionDataAsset.h"

void ULA_MissionLogic::InitializeMission(ULA_MissionDataAsset* InDataAsset, ALA_GameModeBase* InGameMode)
{
    MissionDataAsset = InDataAsset;
    GameMode = InGameMode;
    GameState = GameMode ? GameMode->GetLAGameState() : nullptr;
}

void ULA_MissionLogic::StartPhase(int32 PhaseIndex)
{
    // 자식 클래스에서 구현
}

void ULA_MissionLogic::AdvanceToNextPhase()
{
    // 자식 클래스에서 구현
}

void ULA_MissionLogic::HandleObjectiveProgress(int32 AddCount)
{
    if (!GameState)
        return;

    // Phase가 끝나면 Objective 진행도 증가 불가
    if (GameState->IsCurrentPhaseCompleted())
        return;

    // Objective 진행도 Game State에 반영
    GameState->AddObjectiveProgress(AddCount);


    if (GameState->IsObjectiveCompleted())
    {
        CompleteCurrentPhase();
    }
}

void ULA_MissionLogic::CompleteCurrentPhase()
{
    if (!GameState)
        return;

    if (GameState->IsCurrentPhaseCompleted())
        return;

    // 현재 Phase 종료
    GameState->SetCurrentPhaseCompleted(true);
}
