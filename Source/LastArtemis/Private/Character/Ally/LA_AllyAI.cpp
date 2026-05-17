// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/LA_HUD.h"
#include "BehaviorTree/BlackboardComponent.h"

ALA_AllyAI::ALA_AllyAI()
{

}

void ALA_AllyAI::BeginPlay()
{
    Super::BeginPlay();
    // 현재 월드에서 생성되어 있는 ULA_HUD 타입의 모든 위젯을 검색
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULA_HUD::StaticClass());

    // 위젯이 존재한다면
    if (FoundWidgets.Num() > 0)
    {
        if (ULA_HUD* HUD = Cast<ULA_HUD>(FoundWidgets[0]))
        {
            HUD->RegisterAllyAuto(this);
        }
    }
}

void ALA_AllyAI::SetTargetEnemy(AActor* TargetEnemy)
{
    ALA_AllyAIController* AllyAIController = Cast<ALA_AllyAIController>(GetController());
    if (AllyAIController) return;

    UBlackboardComponent* Blackboard = AllyAIController->GetBlackboardComponent();
}


