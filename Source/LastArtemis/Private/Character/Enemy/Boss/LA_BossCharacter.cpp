// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "Character/LA_BaseCharacter.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"

ALA_BossCharacter::ALA_BossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;

    MaxShield = 100.0f;
    CurrentShield = MaxShield;

    CurrentPhase = EBossPhase::Phase1;
    bIsDead = false; // 안전하게 초기화

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        CharacterTags.AddTag(EnemyTag);
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_None);
    }
}

void ALA_BossCharacter::BeginPlay()
{
    Super::BeginPlay();
}

float ALA_BossCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0.0f) return 0.0f;

    if (CurrentShield > 0.0f)
    {
        CurrentShield -= ActualDamage;
        if (CurrentShield < 0.0f)
        {
            CurrentHealth += CurrentShield; // 초과된 데미지를 체력에 반영
            CurrentShield = 0.0f;
        }
    }
    else
    {
        CurrentHealth -= ActualDamage;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("[보스 피격] 남은 체력: %f / %f"), CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
        return ActualDamage;
    }

    CheckPhaseTransition();

    if (HitMontage)
    {
        PlayAnimMontage(HitMontage);
    }

    return ActualDamage;
}

void ALA_BossCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true; // 중복 사망 방지


    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    // 보스가 죽었으므로 모든 충돌(Collision)을 꺼서 플레이어가 시체를 통과할 수 있게 하거나,
    // 투사체에 더 이상 안 맞도록 설정합니다.
    UCapsuleComponent* MyCapsule = GetCapsuleComponent();
    if (MyCapsule)
    {
        MyCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // AI 컨트롤러 작동 중지 시키기 (죽었는데 계속 총 쏘는 거 방지)
    if (GetController())
    {
        GetController()->StopMovement(); // 이동 중지
        GetController()->UnPossess();    // 컨트롤러 분리 (패턴 작동 중지)
    }

    SetLifeSpan(10.0f);
    UE_LOG(LogTemp, Error, TEXT("[보스 사망] 보스가 처치되었습니다!"));
}

void ALA_BossCharacter::CheckPhaseTransition()
{
    if (bIsDead) return;

    float HealthPercent = CurrentHealth / MaxHealth;

    if (HealthPercent <= 0.33f && CurrentPhase != EBossPhase::Phase3)
    {
        CurrentPhase = EBossPhase::Phase3;
        UE_LOG(LogTemp, Warning, TEXT("[보스 페이즈 변경] >>> PHASE 3 진입 <<<"));
    }
    // 66% 이하 진입 시 페이즈 2로 전환
    else if (HealthPercent <= 0.66f && CurrentPhase == EBossPhase::Phase1)
    {
        CurrentPhase = EBossPhase::Phase2;
        UE_LOG(LogTemp, Warning, TEXT("[보스 페이즈 변경] >>> PHASE 2 진입 <<<"));
    }
}


