// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LA_GameType.generated.h"

// Game Flow State
// UI / 전체 흐름 용
UENUM(BlueprintType)
enum class ELA_GameFlowState : uint8
{
	None,
	MainMenu,
    MissionSelect,
	Playing,
	Paused,
	GameOver,
	GameClear,
};

// 현재 퀘스트 타입
UENUM(BlueprintType)
enum class ELA_MissionType : uint8
{
    None,
    Invasion,    // 침입 퀘스트
    Guard,      // 호위 퀘스트
};

// 현재 Phase 타입
UENUM(BlueprintType)
enum class ELA_PhaseType : uint8
{
    None,
    Combat,     // 전투
    Rest,       // 휴식
    Boss,       // 보스전
};


USTRUCT(BlueprintType)
struct FLA_PhaseData
{
    GENERATED_BODY()

    // 현재 Phase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    ELA_PhaseType PhaseType;

    // 이번 Phase 상황 및 목표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    FText ObjectiveText;

    // 스폰 몬스터 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
    int32 SpawnCount;

    // 필요한 킬 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    int32 RequiredKillCount;

};
