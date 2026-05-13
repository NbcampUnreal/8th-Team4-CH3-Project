    // Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LA_MissionResult.h"
#include "Components/TextBlock.h"  
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void ULA_MissionResult::NativeConstruct()
{
    Super::NativeConstruct();
        
    if (Btn_ToMissionSelect)
    {
        Btn_ToMissionSelect->OnClicked.AddDynamic(this, &ULA_MissionResult::OnMissionSelectClicked);
    }
}

void ULA_MissionResult::DisplayResults(float Time, int32 Score, FString Rank)
{
    // 시간 계산 (초 단위를 분:초로 변환)
    int32 Min = FMath::FloorToInt(Time / 60.f); 
    int32 Sec = FMath::FloorToInt(Time) % 60;

    TargetScore = Score;

    if (Text_ClearTime)
        Text_ClearTime->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Min, Sec)));

    if (Text_TotalScore)
        Text_TotalScore->SetText(FText::AsNumber(Score));

    if (Text_Rank)
        Text_Rank->SetText(FText::FromString(Rank));
}


void ULA_MissionResult::OnMissionSelectClicked()
{
    // 레벨 전환(임무 선택 창으로 이동)
    UGameplayStatics::OpenLevel(this, FName("MainMenu?DirectToMission=1"));
}
