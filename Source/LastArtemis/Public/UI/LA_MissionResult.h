// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_MissionResult.generated.h"


UCLASS()
class LASTARTEMIS_API ULA_MissionResult : public UUserWidget
{
	GENERATED_BODY()

public:
    // 위젯 초기화 시 호출되는 언리얼 기본 함수
    virtual void NativeConstruct() override;

    // 외부에서 데이터를 전달받아 화면에 표시하는 함수
    void DisplayResults(float Time, int32 Score, FString Rank);

    // 버튼 클릭 시 실행될 함수
    UFUNCTION()
    void OnMissionSelectClicked();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    int32 TargetScore = 0;

    // 에디터의 위젯 이름과 변수 이름이 일치하면 자동으로 연결.
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_ClearTime;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    class UTextBlock* Text_TotalScore;  

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Rank;

    UPROPERTY(meta = (BindWidget))
    class UButton* Btn_ToMissionSelect;

};
