// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LA_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../Weapon/LA_WeaponBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "GameMode/LA_GameModeBase.h"
#include "Character/Player/LA_PlayerCharacter.h"
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
    BindAmmo();
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

        // 바인딩 직후 한번 실행해줌
        UpdateHP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
        UpdateShield(HealthComp->GetCurrentShield(), HealthComp->GetMaxShield());
        UE_LOG(LogTemp, Log, TEXT("HUD: Binding & Initial Update Success!"));
    }
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
