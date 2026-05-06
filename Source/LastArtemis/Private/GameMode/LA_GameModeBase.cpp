// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_GameModeBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_MissionDataAsset.h"
#include "GameMode/LA_MissionLogic.h"
#include "LastArtemis/Character/LA_PlayerCharacter.h"
#include "Character/LA_DefaultPlayerController.h"
#include "Kismet/GameplayStatics.h"

/// <게임 흐름 구조>
/// <Main Menu -> Mission Select>
/// 1. 게임 실행 시, MainMenu UI 실행
/// 2. '게임 시작' 선택하면 MissionSelect UI 실행
/// 3. MissionSelect UI에서 원하는 Mission 선택
/// 
/// <Mission Select -> Playing>
/// 4. 선택한 MissionDataAsset을 GameMode에 지정
///     - MissionDataAsset에는 MissionLogicClass가 설정되어 있음
/// 5. GameMode에서 StartMission() 호출하면, MissionLogic 객체가 생성됨
/// 6. Mission Logic은 StartPahse(0)을 호출하여 첫 번째 Phase가 시작됨
///     - 여기서 Mission Logic 파생 클래스는 각자의 Phase 및 Objective 등의 정보를 가져옴
/// 7. Phase가 시작되면 Game State에서 현재 Phase의 정보를 저장
///     - Phase Index, Phase Type, Objective 정보 등
/// 8. Objective 이벤트가 발생하면 GameMode에서 AddObjectiveProgress() 호출 및 MissionLogic에 전달
/// 9. MissionLogic은 GameState의 정보를 업데이트하고, 요구 진행도를 달성하면 현재 Phase 완료 처리
/// 10. 플레이어가 다른 구간 트리거에 진입하면 GameMode의 AdvancedToNextPhase() 호출
/// 11. Mission Logic은 다음 Phase 실행, 모든 Phase가 끝나면 Game Clear 상태로 전환
/// 
/// 
/// </summary>

ALA_GameModeBase::ALA_GameModeBase()
    :
    CurrentMissionLogic(nullptr)
{
    DefaultPawnClass = ALA_PlayerCharacter::StaticClass();
    PlayerControllerClass = ALA_DefaultPlayerController::StaticClass();
    GameStateClass = ALA_GameStateBase::StaticClass();
}


ALA_GameStateBase* ALA_GameModeBase::GetLAGameState() const
{
    return GetGameState<ALA_GameStateBase>();
}

void ALA_GameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 게임 시작 시 Main Menu로 세팅
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::MainMenu);
        LA_GameState->SetMissionType(ELA_MissionType::None);
    }
}

////////////////////////
/// 게음 흐름 제어
////////////////////////

void ALA_GameModeBase::StartNewGame()
{
    // 로비 or 메인 메뉴에서 "새 게임"을 클릭하면 미션 선택 창을 넘어감 
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::MissionSelect);
        LA_GameState->SetMissionType(ELA_MissionType::None);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void ALA_GameModeBase::PauseGame()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Paused);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ALA_GameModeBase::ResumeGame()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Playing);
    }

    // 
    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void ALA_GameModeBase::OnGameOver()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameOver);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ALA_GameModeBase::OnGameClear()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameClear);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

////////////////////////
/// 미션 진행 제어
////////////////////////

void ALA_GameModeBase::StartMission()
{
    ALA_GameStateBase* LA_GameState = GetLAGameState();
    if (!LA_GameState)
        return;

    // 전투가 끝나기 전까지는 로비로 돌아가더라도 Phase 변하지 않음
    if (//LA_GameState->GetGameFlowState() == ELA_GameFlowState::Playing &&
        LA_GameState->GetCurrentPhaseIndex() >= 0)
        return;

    // Data Asset과 Mission Logic 에디터 세팅
    if (!MissionDataAsset || !MissionDataAsset->MissionLogicClass)
        return;

    CurrentMissionLogic = NewObject<ULA_MissionLogic>(
        this,
        MissionDataAsset->MissionLogicClass
    );

    if (!CurrentMissionLogic)
        return;

    // 생성된 Mission Logic 초기화 작업 및 Phase 시작
    CurrentMissionLogic->InitializeMission(MissionDataAsset, this);
    CurrentMissionLogic->StartPhase(0);
}

void ALA_GameModeBase::AddObjectiveProgress(int32 AddCount)
{
    if (CurrentMissionLogic)
    {
        CurrentMissionLogic->HandleObjectiveProgress(AddCount);
    }
}

void ALA_GameModeBase::CompleteCurrentPhase()
{
    if (CurrentMissionLogic)
    {
        CurrentMissionLogic->CompleteCurrentPhase();
    }
}

void ALA_GameModeBase::AdvanceToNextPhase()
{
    if (CurrentMissionLogic)
    {
        CurrentMissionLogic->AdvanceToNextPhase();
    }
}

