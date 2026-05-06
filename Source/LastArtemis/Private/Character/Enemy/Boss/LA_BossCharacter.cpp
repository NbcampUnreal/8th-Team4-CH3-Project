// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "Character/LA_BaseCharacter.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"

ALA_BossCharacter::ALA_BossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 보스 기본 스탯 설정 (기획서 기준)
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;

    MaxShield = 100.0f;
    CurrentShield = MaxShield;

    // 초기 페이즈 설정
    CurrentPhase = EBossPhase::Phase1;

    // 팀 소속 태그 추가 (Team.Enemy)
    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        CharacterTags.AddTag(EnemyTag);
    }

    // 고정형 보스이므로 이동 기능 비활성화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_None);
    }
}

void ALA_BossCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ALA_BossCharacter::TakeDamageCustom(float DamageAmount)
{
    if (bIsDead) return;

    // 부모 클래스의 데미지 처리 로직 실행
    Super::TakeDamageCustom(DamageAmount);

    // 페이즈 전환 조건 검사
    CheckPhaseTransition();

    if (HitMontage)
    {
        PlayAnimMontage(HitMontage);
    }
}

void ALA_BossCharacter::Die()
{
    if (bIsDead) return;

    // 부모 클래스의 사망 로직 실행
    Super::Die();

    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    // 완전히 정지
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    SetLifeSpan(5.0f);
}

void ALA_BossCharacter::CheckPhaseTransition()
{
    // 현재 남은 체력 비율 계산
    float HealthPercent = CurrentHealth / MaxHealth;

    // 33% 이하 진입 시 페이즈 3으로 전환
    if (HealthPercent <= 0.33f && CurrentPhase != EBossPhase::Phase3)
    {
        CurrentPhase = EBossPhase::Phase3;
        // TODO: 3페이즈 광폭화 로직 연동
    }
    // 66% 이하 진입 시 페이즈 2로 전환
    else if (HealthPercent <= 0.66f && CurrentPhase == EBossPhase::Phase1)
    {
        CurrentPhase = EBossPhase::Phase2;
        // TODO: 2페이즈 패턴 변경 로직 연동
    }
}


