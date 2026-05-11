// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_InvasionMissionLogic.h"
#include "GameMode/LA_InvasionMissionDataAsset.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_GameModeBase.h"
#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "UI/LA_GameType.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"

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
        }

        return;
    }

    const FLA_InvasionPhaseData& PhaseData = InvasionDataAsset->PhaseList[PhaseIndex];

    // 현재 Phase 임시 출력
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Green,
            FString::Printf(
                TEXT("Phase Start: %d / Goal: %s / OvjectiveProgress: %d"),
                PhaseIndex,
                *PhaseData.ObjectiveText.ToString(),
                PhaseData.RequiredProgressCount
            )
        );
    }

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
    // 지금은 안 쓰는중
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

// 적 처치 시 킬 카운트 추가
void ULA_InvasionMissionLogic::HandleEnemyKilled(AActor* DeadEnemy)
{
    if (!GameState || !DeadEnemy)
        return;

    if (GameState->IsCurrentPhaseCompleted())
        return;

    ALA_EnemyCharacter* DeadEnemyCharacter = Cast<ALA_EnemyCharacter>(DeadEnemy);
    if (!DeadEnemyCharacter)
        return;

    // 적 tag 검사
    const FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    const FGameplayTag BossTag = FGameplayTag::RequestGameplayTag(FName("Team.Boss"));

    bool bIsEnemyTag = false;
    bool bIsBossTag = false;

    // 처치한 적이 enemy 태그를 가지고 있으면 true
    if (ALA_EnemyCharacter* EnemyCharacter = Cast<ALA_EnemyCharacter>(DeadEnemy))
    {
        bIsEnemyTag = EnemyCharacter->CharacterTags.HasTagExact(EnemyTag);
    }
    // 처치한 적이 boss 태그를 가지고 있으면 true
    else if (ALA_BossCharacter* BossCharacter = Cast<ALA_BossCharacter>(DeadEnemy))
    {
        bIsBossTag = BossCharacter->CharacterTags.HasTagExact(BossTag);
    }

    const ELA_PhaseType CurrentPhaseType = GameState->GetCurrentPhaseType();

    switch (CurrentPhaseType)
    {
        case ELA_PhaseType::Combat:
        {
            // Combat Phase에서는 enemy를 처치할 시 카운트
            // enemy 태그 붙이기
            if (!bIsEnemyTag)
                return;

            HandleObjectiveProgress(1);
            break;
        }

        case ELA_PhaseType::Boss:
        {
            // Boss Phase에서는 Boss 태그가 있는 적만 카운트
            // Boss 태그
            if (!bIsBossTag)
                return;

            HandleObjectiveProgress(1);
            break;
        }

        default:
            break;
    }
}


// 미션별 임무 클리어 로직 구현
void ULA_InvasionMissionLogic::HandleInvasionObjective(const FLA_InvasionPhaseData& PhaseData)
{
    // 지금은 킬 카운트만 증가
    if (PhaseData.RequiredProgressCount <= 0)
    {
        CompleteCurrentPhase();
    }

    //switch (PhaseData.ObjectiveType)
    //{
    //case ELA_InvasionObjectiveType::None:
    //    CompleteCurrentPhase();
    //    break;

    //case ELA_InvasionObjectiveType::DisableTurret:
    //    // 자동 포탑 무력화 로직 구현
    //    break;

    //case ELA_InvasionObjectiveType::ControlPassage:
    //    // 핵심 연결 통로 제어 로직 구현
    //    break;

    //case ELA_InvasionObjectiveType::BreakDefenseSystem:
    //    // 방어 시스템 돌파 로직 구현
    //    break;

    //case ELA_InvasionObjectiveType::KillBoss:
    //    // 보스 처치 목표 로직 구현
    //    break;

    //default:
    //    break;
    //}
}
