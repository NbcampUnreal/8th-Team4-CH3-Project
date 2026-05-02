// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_GameMode.h"
#include "LA_BaseCharacter.h"
#include "LA_GameState.h"

ALA_GameMode::ALA_GameMode()
{
	DefaultPawnClass = ALA_BaseCharacter::StaticClass();
	// 플레이어 컨트롤러
	GameStateClass = ALA_GameState::StaticClass();
}

void ALA_GameMode::StartGame()
{
}

void ALA_GameMode::OnGameOver()
{
}

void ALA_GameMode::OnGameClear()
{
}


