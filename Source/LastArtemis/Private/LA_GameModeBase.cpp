// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_GameModeBase.h"
#include "LastArtemis\Character\LA_PlayerCharacter.h"
#include "LA_DefaultPlayerController.h"
#include "LA_GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ALA_GameModeBase::ALA_GameModeBase()
{
	DefaultPawnClass = ALA_PlayerCharacter::StaticClass();
	PlayerControllerClass = ALA_DefaultPlayerController::StaticClass();
	GameStateClass = ALA_GameStateBase::StaticClass();
}

void ALA_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::MainMenu);
	}
}

void ALA_GameModeBase::StartNewGame()
{
	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::Playing);
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		// HUD 표시
		// 입력 모드를 Game Only로 변경
		// 웨이브 시작
	}
}

void ALA_GameModeBase::LoadSaveGame()
{

}

void ALA_GameModeBase::StartWave(int32 WaveIndex)
{

}

void ALA_GameModeBase::EndWave()
{

}

void ALA_GameModeBase::PauseGame()
{
	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::Paused);
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		// 일시 정지 UI
		// 입력 모드 UI Only로 변경
	}
}

void ALA_GameModeBase::ResumeGame()
{
	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::Playing);
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		// 일시 정지 UI 닫기
		// 입력 모드 Game Only로 변경
	}
}

void ALA_GameModeBase::OnGameOver()
{
	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::GameOver);
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		// 게임 오버 UI
	}
}

void ALA_GameModeBase::OnGameClear()
{
	if (ALA_GameStateBase* GameState = GetGameState<ALA_GameStateBase>())
	{
		GameState->SetGameFlowState(ELA_GameFlowState::GameClear);
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		// 게임 클리어 UI
	}
}

