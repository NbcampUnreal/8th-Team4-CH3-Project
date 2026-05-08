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

// 현재 미션 타입
UENUM(BlueprintType)
enum class ELA_MissionType : uint8
{
    None,
    Invasion,    // 침투 미션
    Guard,      // 호위 미션, 아직 미구현
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


// 침투 미션의 임무 목록
UENUM(BlueprintType)
enum class ELA_InvasionObjectiveType : uint8
{
    None,

    DisableTurret,          // 자동 포탑 무력화
    ControlPassage,         // 핵심 연결 통로 제어
    BreakDefenseSystem,     // 방어 시스템 돌파
    KillBoss                // 보스 처치
};

// 침투 미션 데이터 구조체
USTRUCT(BlueprintType)
struct FLA_InvasionPhaseData
{
    GENERATED_BODY()

    // 현재 Phase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    ELA_PhaseType PhaseType = ELA_PhaseType::None;

    // 이번 Phase 상황 및 목표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    ELA_InvasionObjectiveType ObjectiveType = ELA_InvasionObjectiveType::None;

    // 현재 Phase 상황 및 목표 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    FText ObjectiveText;

    // 목표 킬 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
    int32 RequiredProgressCount = 0;
};
