// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/LA_MissionLogic.h"
#include "UI/LA_Gametype.h"
#include "LA_InvasionMissionLogic.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class LASTARTEMIS_API ULA_InvasionMissionLogic : public ULA_MissionLogic
{
	GENERATED_BODY()

public:
    virtual void StartPhase(int32 PhaseIndex) override;
    virtual void AdvanceToNextPhase() override;

protected:
    // 침투 미션 세부 기능 구현
    void HandleInvasionObjective(const FLA_InvasionPhaseData& PhaseData);
};
