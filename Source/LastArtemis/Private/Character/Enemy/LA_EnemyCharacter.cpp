#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

ALA_EnemyCharacter::ALA_EnemyCharacter()
{
	AIControllerClass = ALA_EnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 80.0f;
	CurrentHealth = MaxHealth;

	MaxShield = 30.0f;
	CurrentShield = MaxShield;

	AttackPower = 15.0f;
	Defense = 3.0f;

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        GameplayTags.AddTag(EnemyTag);
    }
}

void ALA_EnemyCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(GameplayTags);
}

float ALA_EnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bIsDead) return 0.0f;

    if (ActualDamage > 0.0f && HitMontage)
    {
        PlayAnimMontage(HitMontage);
    }

    return ActualDamage;
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

