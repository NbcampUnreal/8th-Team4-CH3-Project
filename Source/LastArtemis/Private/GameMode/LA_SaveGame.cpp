// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_SaveGame.h"
#include "GameMode/LA_MissionDataAsset.h"

ULA_SaveGame::ULA_SaveGame()
    :
    SavedMissionDataAsset(nullptr),
    SavedPhaseIndex(-1),
    SavedPlayerLocation(FVector::ZeroVector),
    SavedPlayerRotation(FRotator::ZeroRotator),
    SavedGold(0),
    SavedScore(0)
{

}
