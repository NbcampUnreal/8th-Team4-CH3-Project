// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_GameStateBase.h"

ALA_GameStateBase::ALA_GameStateBase()
	:
	CurrentGameFlowState(ELA_GameFlowState::None)
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

