#include "LA_BaseCharacter.h"
#include "Components/CapsuleComponent.h"

ALA_BaseCharacter::ALA_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	
	MaxShield = 50.0f;
	CurrentShield = MaxShield;

	AttackPower = 10.0f;
	Defense = 5.0f;
	//CurrentWeapon = nullptr;
	bIsDead = false;
}

void ALA_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	/*
	if (DefaultWeaponClass)
	{
		EquipWeapon(DefaultWeaponClass);
	}*/
	
}


void ALA_BaseCharacter::TakeDamageCustom(float DamageAmount)
{
	if (bIsDead) return;
	
	float FinalDamage = FMath::Max(DamageAmount - Defense, 0.0f);
	
	if (CurrentShield > 0.0f)
	{
		float DamageToShield = FMath::Min(FinalDamage, CurrentShield);
		CurrentShield -= DamageToShield;
		FinalDamage -= DamageToShield;
		
		if (OnShieldChanged.IsBound())
		{
			OnShieldChanged.Broadcast(CurrentShield);
		}
	}

	if (FinalDamage > 0.0f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
		
		if (OnHealthChanged.IsBound())
		{
			OnHealthChanged.Broadcast(CurrentHealth);
		}
	}
	
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ALA_BaseCharacter::Die()
{
	bIsDead = true;

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (OnDeath.IsBound())
	{
		OnDeath.Broadcast();
	}
}
/*
void ALA_BaseCharacter::EquipWeapon(TSubclassOf<ALA_Weapon> WeaponClass)
{
	if (GetWorld() && WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		CurrentWeapon = GetWorld()->SpawnActor<AAWeapon>(WeaponClass, SpawnParams);
	}
}

float ALA_BaseCharacter::GetAttackPower() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->AttackPower;
	}
	return AttackPower;
}*/
