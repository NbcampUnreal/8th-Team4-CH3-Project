#include "LA_EnemyCharacter.h"

#include "AI/LA_EnemyController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

ALA_EnemyCharacter::ALA_EnemyCharacter()
{
	AIControllerClass = ALA_EnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Spawned;
	
	PrimaryActorTick.bCanEverTick = true;
	
	MaxHealth = 80.0f;
	CurrentHealth = MaxHealth;
    
	MaxShield = 30.0f;
	CurrentShield = MaxShield;

	AttackPower = 15.0f;
	Defense = 3.0f;
}

void ALA_EnemyCharacter::TakeDamageCustom(float DamageAmount)
{
	
	if (bIsDead) return;
	
	Super::TakeDamageCustom(DamageAmount);
	
	// 피격 애니메이션 재생
	if (HitMontage)
	{
		PlayAnimMontage(HitMontage);
	}
}

void ALA_EnemyCharacter::Die()
{
	if (bIsDead) return;
	
	// 부모 클래스의 사망 로직(충돌 비활성화 등) 실행
	Super::Die();
	
	// 사망 애니메이션 재생
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	// 사망 시 이동 및 회전 완전 정지
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
	
	SetLifeSpan(5.0f);
}

void ALA_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

