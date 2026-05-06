// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UI/LA_GameType.h"
#include "LA_GameStateBase.generated.h"

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ALA_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALA_GameStateBase();

	// game flow state
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameFlowState(ELA_GameFlowState NewState);
	UFUNCTION(BlueprintPure, Category = "Game State")
	ELA_GameFlowState GetGameFlowState() const;

    // 미션
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetMissionType(ELA_MissionType NewMissionType);
    UFUNCTION(BlueprintPure, Category = "Mission")
    ELA_MissionType GetMissionType() const;

    // Phase
    UFUNCTION(BlueprintCallable, Category = "Phase")
    void SetCurrentPhase(int32 NewPhaseIndex, const FLA_PhaseData& PhaseData);

    UFUNCTION(BlueprintCallable, Category = "Phase")
    void AddKillCount(int32 AddCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Phase")
    void ResetKillCount();

    UFUNCTION(BlueprintPure, Category = "Phase")
    bool IsKillObjectiveCompleted() const;

    UFUNCTION(BlueprintPure, Category = "Phase")
    int32 GetCurrentPhaseIndex() const;

    UFUNCTION(BlueprintPure, Category = "Phase")
    ELA_PhaseType GetCurrentPhaseType() const;

    UFUNCTION(BlueprintPure, Category = "Phase")
    int32 GetCurrentKillCount() const;

    UFUNCTION(BlueprintPure, Category = "Phase")
    int32 GetRequiredKillCount() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	ELA_GameFlowState CurrentGameFlowState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ELA_MissionType CurrentMissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    ELA_PhaseType CurrentPhaseType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    int32 CurrentPhaseIndex;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    FText CurrentObjectiveText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    int32 CurrentKillCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    int32 RequiredKillCount;

};
