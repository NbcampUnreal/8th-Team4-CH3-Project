// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LA_MissionLogic.generated.h"

class ALA_GameModeBase;
class ALA_GameStateBase;
class ULA_MissionDataAsset;

UCLASS(Abstract)
class LASTARTEMIS_API ULA_MissionLogic : public UObject
{
	GENERATED_BODY()

public:
    // Game Mode에서 호출
    virtual void InitializeMission(ULA_MissionDataAsset* InDataAsset, ALA_GameModeBase* InGameMode);

    // 특정 번호의 Phase 실행
    virtual void StartPhase(int32 PhaseIndex);
    // 다음 Phase로 넘어감
    virtual void AdvanceToNextPhase();

    // Objective를 GameState에서 관리
    // Objective 달성하면 Phase 완료 처리
    virtual void HandleObjectiveProgress(int32 AddCount);
    // Phase 완료 처리
    virtual void CompleteCurrentPhase();

protected:
    UPROPERTY()
    TObjectPtr<ALA_GameModeBase> GameMode;

    UPROPERTY()
    TObjectPtr<ALA_GameStateBase> GameState;

    UPROPERTY()
    TObjectPtr<ULA_MissionDataAsset> MissionDataAsset;
};
