// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UI/LA_GameType.h"
#include "LA_GameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMissionStatusChanged, ULA_MissionDataAsset*, MissionData, int32, PhaseIndex, int32, CurrentCount);
/**
 *
 */
UCLASS(Blueprintable)
class LASTARTEMIS_API ALA_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
    ALA_GameStateBase();

    ////////////////////////
    /// 게임 흐름 제어
    ////////////////////////
    
    // 게임 흐름 설정
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void SetGameFlowState(ELA_GameFlowState NewState);
    // 현재 게임 흐름 상태 반환
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    ELA_GameFlowState GetGameFlowState() const;

    // UI 업데이트
    FOnMissionStatusChanged OnMissionStatusChanged;

    ////////////////////////
    /// 미션 정보
    ////////////////////////
    
    // 진행할 미션 설정
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetMissionType(ELA_MissionType NewMissionType);
    // 현재 미션 종류 반환(Invasion, Guard 등)
    UFUNCTION(BlueprintCallable, Category = "Mission")
    ELA_MissionType GetMissionType() const;

    ////////////////////////
    /// Phase / Objective
    ////////////////////////

    // 새로운 Phase가 시작될 때마다 Objective 갱신
    void SetCurrentPhaseInfo(
        int32 NewPhaseIndex,            // 현재 Phase의 번호
        ELA_PhaseType NewPhaseType,     // Phase 타입
        const FText& NewObjectiveText,  // UI에 출력될 Objective
        int32 NewRequiredProgressCount  // 목표 Objective 진행도
    );

    // 현재 Phase Index 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Phase")
    int32 GetCurrentPhaseIndex() const;
    // 현재 Phase 타입 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Phase")
    ELA_PhaseType GetCurrentPhaseType() const;
    // Phase 완료 여부 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Phase")
    bool IsCurrentPhaseCompleted() const;
    // 현재 Phase의 Objective 완료 여부 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Objective")
    bool IsObjectiveCompleted() const;

    // 현재 Objective 진행도 증가
    void AddObjectiveProgress(int32 AddCount);
    // 현재 Objective 진행도 초기화
    void ResetObjectiveProgress();
    // Phase 완료 여부 설정
    void SetCurrentPhaseCompleted(bool bCompleted);

    // 현재 Objective 진행도 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Objective")
    int32 GetCurrentProgressCount() const;
    // 목표 Objective 진행도 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Objective")
    int32 GetRequiredProgressCount() const;
    // 진행중인 Objective 반환
    UFUNCTION(BlueprintPure, Category = "Mission|Objective")
    FText GetCurrentObjectiveText() const;

    // 타이머 시간 초기화
    UFUNCTION(BlueprintCallable, Category = "Game Timer")
    void ResetElapsedGameTime();
    // 시간 경과
    UFUNCTION(BlueprintCallable, Category = "Game Timer")
    void AddElapsedGameTime(int32 AddSeconds);
    // 경과된 시간 리턴
    UFUNCTION(BlueprintPure, Category = "Game Timer")
    int32 GetElapsedGameTime() const;
    // 경과된 시간 텍스트 리턴
    UFUNCTION(BlueprintPure, Category = "Game Timer")
    FText GetElapsedGameTimeText() const;
    // 현재 저장된 시간
    UFUNCTION(BlueprintCallable, Category = "Game Timer")
    void SetElapsedGameTime(int32 NewElapsedGameTime);

protected:
    // 현재 Game Flow State(Main Menu, Playing, Paused 등)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
    ELA_GameFlowState CurrentGameFlowState;

    // 현재 플레이 중인 미션(Invasion, Guard 등)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
    ELA_MissionType CurrentMissionType;

    // Phase 완료 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Phase")
    bool bCurrentPhaseCompleted;
    // 현재 진행 중인 미션의 Phase 번호
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Phase")
    int32 CurrentPhaseIndex;
    // 현재 Phase 타입(Rest, Combat, Boss 등)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Phase")
    ELA_PhaseType CurrentPhaseType;

    // 현재 Phase의 Objective
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
    FText CurrentObjectiveText;
    // 현재 Objective의 진행도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
    int32 CurrentProgressCount;
    // 목표 Objective의 진행도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
    int32 RequiredProgressCount;

    // 경과 시간
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Timer")
    int32 ElapsedGameTime;
};
