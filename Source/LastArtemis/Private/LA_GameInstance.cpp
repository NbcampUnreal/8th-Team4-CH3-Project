#include "LA_GameInstance.h"

ULA_GameInstance::ULA_GameInstance()
	:
	PlayerLevel(1),
	CurrentExp(0),
	ExpToNextLevel(100),
	TotalGold(0),
	TotalScore(0)
{

}

////////////////////////////////////////////////////////////////////////
/// 보상 관련 로직
////////////////////////////////////////////////////////////////////////

void ULA_GameInstance::AddReward(int32 ExpReward, int32 GoldReward, int32 ScoreReward)
{
	AddExp(ExpReward);
	TotalGold += GoldReward;
	TotalScore += ScoreReward;
}

void ULA_GameInstance::AddExp(int32 ExpAmount)
{
	if (ExpAmount <= 0)
		return;

	CurrentExp += ExpAmount;
	CheckLevelUp();
}

void ULA_GameInstance::CheckLevelUp()
{
	while (CurrentExp >= ExpToNextLevel)
	{
		CurrentExp -= ExpToNextLevel;
		LevelUp();
	}
}

void ULA_GameInstance::LevelUp()
{
	++PlayerLevel;

	// 플레이어 스탯 증가
	{

	}
}

void ULA_GameInstance::ResetPlayerData()
{
	PlayerLevel = 1;
	CurrentExp = 0;
	ExpToNextLevel = 0;
	TotalGold = 0;
}


