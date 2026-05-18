// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LA_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../Weapon/LA_WeaponBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_GameModeBase.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Components/HorizontalBox.h"
#include "Character/Player/Component/LA_HealthComponent.h"

void ULA_HUD::NativeConstruct()
{
    Super::NativeConstruct();

    GameState = Cast<ALA_GameStateBase>(GetWorld()->GetGameState());

    if (GameState)
    {
        // 델리게이트에 업데이트 함수 연결
        GameState->OnMissionStatusChanged.AddDynamic(this, &ULA_HUD::UpdateMission);

        ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(GetWorld()->GetAuthGameMode());
        ULA_MissionDataAsset* CurrentData = GM ? GM->GetMissionDataAsset() : nullptr;

        // 초기화 시점에 현재 데이터를 한 번 반영
        UpdateMission(
            CurrentData,
            GameState->GetCurrentPhaseIndex(),
            GameState->GetCurrentProgressCount()
        );
    }

    BindHealth();
    BindContamination();
    BindAmmo();

    if (HorizontalBox_Ally1) HorizontalBox_Ally1->SetVisibility(ESlateVisibility::Collapsed);
    if (HorizontalBox_Ally2) HorizontalBox_Ally2->SetVisibility(ESlateVisibility::Collapsed);
}

void ULA_HUD::RegisterAllyAuto(ALA_AllyAI* NewAlly)
{
    if (!NewAlly) return;
    if (NewAlly == Ally1Actor || NewAlly == Ally2Actor) return;

    ULA_HealthComponent* HealthComp = NewAlly->FindComponentByClass<ULA_HealthComponent>();
    if (!HealthComp) return;

    // 1번 슬롯이 비어있다면
    if (!Ally1Actor)
    {
        Ally1Actor = NewAlly;
        // 박스 표시
        if (HorizontalBox_Ally1) HorizontalBox_Ally1->SetVisibility(ESlateVisibility::Visible);

        // HP 변화 시 UpdateAlly1HP 이벤트 호출되도록 바인딩
        HealthComp->OnHealthChanged.AddUObject(this, &ULA_HUD::UpdateAlly1HP);

        // 초기값 전송
        UpdateAlly1HP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
    }
    // 2번 슬롯이 비어있다면
    else if (!Ally2Actor)
    {
        Ally2Actor = NewAlly;
        // 박스 표시
        if (HorizontalBox_Ally2) HorizontalBox_Ally2->SetVisibility(ESlateVisibility::Visible);

        // HP 변화 시 UpdateAlly2HP 이벤트 호출되도록 바인딩
        HealthComp->OnHealthChanged.AddUObject(this, &ULA_HUD::UpdateAlly2HP);

        // 초기값 전송
        UpdateAlly2HP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
    }
    else
    {
        // 둘 다 꽉 찼으면 아무것도 안 함
        UE_LOG(LogTemp, Warning, TEXT("HUD: All Ally Slots are Full!"));
    }
}

void ULA_HUD::BindHealth()
{
    // OnHealthChanged에 UpdateHP 함수 바인딩
    ACharacter* PlayerCharacter =
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    ULA_HealthComponent* HealthComp = PlayerCharacter->FindComponentByClass<ULA_HealthComponent>();

    if (HealthComp)
    {
        HealthComp->OnHealthChanged.AddUObject(this, &ULA_HUD::UpdateHP);
        HealthComp->OnShieldChanged.AddUObject(this, &ULA_HUD::UpdateShield);

        // 바인딩 직후 초기화
        UpdateHP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
        UpdateShield(HealthComp->GetCurrentShield(), HealthComp->GetMaxShield());

    }

    // 2. 아군 AI 바인딩
    TArray<AActor*> FoundAllies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_AllyAI::StaticClass(), FoundAllies);

    for (int32 i = 0; i < FoundAllies.Num(); ++i)
    {
        ALA_AllyAI* Ally = Cast<ALA_AllyAI>(FoundAllies[i]);
        if (!Ally) continue;

        ULA_HealthComponent* AllyHealth = Ally->FindComponentByClass<ULA_HealthComponent>();
        if (!AllyHealth) continue;

        // i값(0 또는 1)에 따라 서로 다른 위젯 슬롯에 바인딩
        if (i == 0)
        {
            AllyHealth->OnHealthChanged.AddUObject(this, &ULA_HUD::UpdateAlly1HP);
            UpdateAlly1HP(AllyHealth->GetCurrentHealth(), AllyHealth->GetMaxHealth());
        }
        else if (i == 1)
        {
            AllyHealth->OnHealthChanged.AddUObject(this, &ULA_HUD::UpdateAlly2HP);
            UpdateAlly2HP(AllyHealth->GetCurrentHealth(), AllyHealth->GetMaxHealth());
        }
    }
}

void ULA_HUD::BindContamination()
{
    ACharacter* PlayerCharacter =
    UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    ULA_HealthComponent* HealthComp = PlayerCharacter->FindComponentByClass<ULA_HealthComponent>();

    if (HealthComp)
    {
        HealthComp->OnContaminationChanged.AddUObject(this, &ULA_HUD::UpdateContamination);

        // 바인딩 직후 초기화
        UpdateContamination(HealthComp->GetCurrentContamination(), HealthComp->GetMaxContamination());

    }
}

void ULA_HUD::BindQuickSlot()
{
}

void ULA_HUD::BindAmmo()
{
    ALA_PlayerCharacter* PlayerCharacter =
        Cast<ALA_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (PlayerCharacter)
    {
        // 캐릭터의 델리게이트 구독
        PlayerCharacter->OnAmmoChangedSignature.AddUObject(this, &ULA_HUD::UpdateAmmo);
    }
}
