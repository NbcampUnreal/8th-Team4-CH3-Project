// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LA_GameInstance.generated.h"


UCLASS()
class LASTARTEMIS_API ULA_GameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	ULA_GameInstance();

public:
	// 플레이어 현재 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
	int32 PlayerLevel;
	// 현재 경험치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
	int32 CurrentExp;
	// 레벨 업에 필요한 경험치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
	int32 ExpToNextLevel;
	// 현재 골드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
	int32 TotalGold;
	// 몬스터 처치 점수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Data")
	int32 TotalScore;

	// 몬스터 처치 시 보상 추가
	UFUNCTION(BlueprintCallable)
	void AddReward(int32 ExpReward, int32 GoldReward, int32 ScoreReward);
	// 게임 데이터 초기화
	UFUNCTION(BlueprintCallable)
	void ResetPlayerData();

protected:
	////////////////////
	// 경험치 관련 로직
	////////////////////
	void AddExp(int32 ExpAmount);	// 경험치 증가
	void CheckLevelUp();			// 레벨업 체크
	void LevelUp();					// 레벨 업
};
