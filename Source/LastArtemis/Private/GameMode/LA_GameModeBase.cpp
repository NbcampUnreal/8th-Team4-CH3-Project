// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_GameModeBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_MissionDataAsset.h"
#include "GameMode/LA_MissionLogic.h"
#include "GameMode/LA_GameInstance.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Character/LA_DefaultPlayerController.h"
#include "UI/LA_HUD.h"
#include "UI/LA_MissionResult.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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
    MissionDataAsset(nullptr),
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

    ULA_GameInstance* LA_GameInstance = GetGameInstance<ULA_GameInstance>();

    // Game Instance에 선택된 미션 데이터가 존재하는지 확인
    if (LA_GameInstance && LA_GameInstance->GetSelectedMission())
    {
        MissionDataAsset = LA_GameInstance->GetSelectedMission();

        if (ALA_GameStateBase* LA_GameState = GetLAGameState())
        {
            LA_GameState->SetGameFlowState(ELA_GameFlowState::Playing);
        }

        // HUD 생성
        APlayerController* PC = GetWorld()->GetFirstPlayerController();

        if (PC)
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;

            PC->SetIgnoreLookInput(false);
            PC->SetIgnoreMoveInput(false);
        }

        if (PC && HUDClass)
        {
            ULA_HUD* NewHUD = CreateWidget<ULA_HUD>(PC, HUDClass);
            if (NewHUD)
            {
                NewHUD->AddToViewport();
            }
        }

        UGameplayStatics::SetGamePaused(GetWorld(), false);
        
        // 미션 시작
        StartMission();

        // 컨티뉴
        if (LA_GameInstance->CheckPointData.IsValid() && 
            LA_GameInstance->CheckPointData.MissionId == LA_GameInstance->GetSelectedMissionId())
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                PlayerPawn->SetActorLocationAndRotation(
                    LA_GameInstance->CheckPointData.PlayerLocation,
                    LA_GameInstance->CheckPointData.PlayerRotation,
                    false,
                    nullptr,
                    ETeleportType::TeleportPhysics
                );

                if (PC)
                {
                    PC->SetControlRotation(LA_GameInstance->CheckPointData.PlayerRotation);
                }
                
                UE_LOG(LogTemp, Log, TEXT("GameMode: 세이브 데이터 위치로 플레이어 복구 완료."));
            }
        }

        return;
    }

    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::MainMenu);
        LA_GameState->SetMissionType(ELA_MissionType::None);
    }
}

////////////////////////
/// 게임 흐름 제어
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

void ALA_GameModeBase::LoadSavedGame()
{
    // 메인 메뉴에서 "불러오기" 클릭 시 저장된 미션, Rest Phase Index, 체크포인트 위치 복구
    ULA_GameInstance* LA_GameInstance = GetGameInstance<ULA_GameInstance>();
    if (!LA_GameInstance)
        return;

    LA_GameInstance->LoadGameData();

    if (!LA_GameInstance->GetSelectedMission())
        return;

    MissionDataAsset = LA_GameInstance->GetSelectedMission();

    StartMission();

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && LA_GameInstance->CheckPointData.IsValid())
    {
        PlayerPawn->SetActorLocation(LA_GameInstance->CheckPointData.PlayerLocation);
        PlayerPawn->SetActorRotation(LA_GameInstance->CheckPointData.PlayerRotation);
    }
}

void ALA_GameModeBase::PauseGame()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Paused);
    }

    // 일시 정지은 타이머에서 제외
    StopGameTimer();

    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ALA_GameModeBase::ResumeGame()
{
    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::Playing);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), false);

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }

    // 이어서 측정해야 되므로 false
    StartGameTimer(false);
}

void ALA_GameModeBase::OnGameOver()
{
    StopGameTimer();

    if (ALA_GameStateBase* LA_GameState = GetLAGameState())
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameOver);
    }

    if (GameOverWidgetClass)
    {
        APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        if (PC)
        {
            CurrentGameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
            if (CurrentGameOverWidget)
            {
                CurrentGameOverWidget->AddToViewport();

                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(CurrentGameOverWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true; // 마우스 커서 활성화
            }
        }
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);

    ShowGameOverUI();
}

void ALA_GameModeBase::ShowGameOverUI()
{
    if (!GameOverWidgetClass)
        return;

    APlayerController* LA_PlayerController = GetWorld()->GetFirstPlayerController();
    if (!LA_PlayerController)
        return;

    CurrentGameOverWidget = CreateWidget<UUserWidget>(LA_PlayerController, GameOverWidgetClass);
    if (!CurrentGameOverWidget)
        return;

    CurrentGameOverWidget->AddToViewport();

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(CurrentGameOverWidget->TakeWidget());
    LA_PlayerController->SetInputMode(InputMode);
    LA_PlayerController->bShowMouseCursor = true;

    LA_PlayerController->SetIgnoreLookInput(true);
    LA_PlayerController->SetIgnoreMoveInput(true);
}

void ALA_GameModeBase::OnGameClear()
{
    ULA_GameInstance* LA_GameInstance = GetGameInstance<ULA_GameInstance>();
    ALA_GameStateBase* LA_GameState = GetLAGameState(); // GameState 가져오기

    // 미션 결과 데이터 SaveGame에 저장
    if (LA_GameInstance && LA_GameState)
    {
        const int32 FinalTime = LA_GameState->GetElapsedGameTime(); // 플레이 시간
        const int32 TargetClearTime = 1000;
        const int32 TimeBonus = FMath::Max(0, TargetClearTime - FinalTime) / 3;

        LA_GameInstance->AddScore(TimeBonus);

        const int32 FinalScore = LA_GameInstance->TotalScore;       // 최종 점수
        const FString FinalRank = TEXT("S");                        // 최종 랭크

        LA_GameInstance->SaveMissionResultData(FinalTime, FinalScore, FinalRank);

        HandleMissionComplete(static_cast<float>(FinalTime), FinalScore, FinalRank);
    }

    StopGameTimer();

    if (LA_GameState)
    {
        LA_GameState->SetGameFlowState(ELA_GameFlowState::GameClear);
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);

}

void ALA_GameModeBase::HandleMissionComplete(float FinalTime, int32 FinalScore, FString FinalRank)
{
    if (ResultWidgetClass)
    {
        // 위젯 생성
        CurrentResultWidget = CreateWidget<ULA_MissionResult>(GetWorld(), ResultWidgetClass);

        if (CurrentResultWidget)
        {
            // 데이터 전달
            CurrentResultWidget->DisplayResults(FinalTime, FinalScore, FinalRank);

            // 화면에 표시
            CurrentResultWidget->AddToViewport();

            // 입력 모드 설정
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(CurrentResultWidget->TakeWidget());
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
    }
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

    // 새 미션 시작 시 게임 시간 0초부터 시작
    StartGameTimer(true);

    int32 StartPhaseIndex = 0;

    if (ULA_GameInstance* LA_GameInstance = GetGameInstance<ULA_GameInstance>())
    {
        const FPrimaryAssetId CurrentMissionId = MissionDataAsset->GetPrimaryAssetId();


        // 체크 포인트 데이터와 선택한 데이터가 같은 MissionId를 가지고 있으면 저장된 Phase 시작
        if (
            LA_GameInstance->CheckPointData.IsValid() &&
            LA_GameInstance->CheckPointData.MissionId == CurrentMissionId
            )
        {
            StartPhaseIndex = LA_GameInstance->CheckPointData.PhaseIndex;
            LA_GameState->SetElapsedGameTime(LA_GameInstance->CheckPointData.ElapsedGameTime);
        }
    }

    // 선택된 MissionDataAsset을 기반으로 MissionLogic 생성 후 첫 번째 Phase 시작
    CurrentMissionLogic->StartPhase(StartPhaseIndex);
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

void ALA_GameModeBase::NotifyEnemyKilled(AActor* DeadEnemy)
{
    if (!CurrentMissionLogic || !DeadEnemy)
        return;

    CurrentMissionLogic->HandleEnemyKilled(DeadEnemy);
}

ULA_MissionDataAsset* ALA_GameModeBase::GetMissionDataAsset()
{
    return MissionDataAsset;
}

////////////////////////
/// 타이머
////////////////////////

// 게임을 처음 시작할 때는 true
// 게임을 재개할 때는 false
void ALA_GameModeBase::StartGameTimer(bool bResetTime)
{
    ALA_GameStateBase* LA_GameState = GetLAGameState();
    if (!LA_GameState)
        return;

    if (bResetTime)
    {
        LA_GameState->ResetElapsedGameTime();
    }

    GetWorldTimerManager().ClearTimer(GameTimerHandle);

    GetWorldTimerManager().SetTimer(
        GameTimerHandle,
        this,
        &ALA_GameModeBase::UpdateGameTimer,
        1.0f,
        true
    );
}

void ALA_GameModeBase::StopGameTimer()
{
    GetWorldTimerManager().ClearTimer(GameTimerHandle);
}

// 게임 플레이 중에만 타이머 업데이트
void ALA_GameModeBase::UpdateGameTimer()
{
    ALA_GameStateBase* LA_GameState = GetLAGameState();
    if (!LA_GameState)
    {
        StopGameTimer();
        return;
    }

    if (LA_GameState->GetGameFlowState() != ELA_GameFlowState::Playing)
        return;

    LA_GameState->AddElapsedGameTime(1);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            1000,
            1.0f,
            FColor::Green,
            FString::Printf(
                TEXT("Play Time: %s"),
                *LA_GameState->GetElapsedGameTimeText().ToString()
            )
        );
    }
}

