#include "Character/LA_BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ALA_BaseCharacter::ALA_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

    MaxContamination = 100;
    Contamination = 0;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	MaxShield = 50.0f;
	CurrentShield = MaxShield;

	AttackPower = 10.0f;
	Defense = 5.0f;
	//CurrentWeapon = nullptr;
	bIsDead = false;
}

float ALA_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bIsDead || ActualDamage <= 0.0f) return 0.0f;

    float FinalDamage = FMath::Max(ActualDamage - Defense, 0.0f);
    if (FinalDamage <= 0.0f) return 0.0f;

    if (CurrentShield > 0.0f)
    {
        float DamageToShield = FMath::Min(FinalDamage, CurrentShield);
        CurrentShield -= DamageToShield;
        FinalDamage -= DamageToShield;

        if (OnShieldChanged.IsBound()) OnShieldChanged.Broadcast(CurrentShield);
    }

    if (FinalDamage > 0.0f)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
        if (OnHealthChanged.IsBound()) OnHealthChanged.Broadcast(CurrentHealth);

        if (HitSound)
        {
            // 적의 현재 위치(GetActorLocation)에서 사운드를 3D로 입체감 있게 터트립니다.
            UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
        }
    }

    if (CurrentHealth <= 0.0f) Die();

    return ActualDamage;
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

		//if (OnShieldChanged.IsBound())
		//{
		//	OnShieldChanged.Broadcast(CurrentShield);
		//}
	}

	if (FinalDamage > 0.0f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);

		//if (OnHealthChanged.IsBound())
		//{
		//	OnHealthChanged.Broadcast(CurrentHealth);
		//}
	}

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ALA_BaseCharacter::Die()
{
    if (bIsDead) return;
	bIsDead = true;

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

    if (OnDeath.IsBound()) OnDeath.Broadcast();
}

void ALA_BaseCharacter::IncreaseContamination(float Amount)
{
    Contamination = FMath::Clamp(Contamination + Amount, 0.0f, MaxContamination);

    if (Contamination >= MaxContamination)
    {
        // 오염도가 최대치일 때의 페널티 (예: 지속 데미지 등)를 여기에 구현
    }
}

void ALA_BaseCharacter::PlayAttackMontage()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        // 몽타주 재생 (재생 속도 1.0f)
        AnimInstance->Montage_Play(AttackMontage, 1.0f);
    }
}

void ALA_BaseCharacter::ReduceShieldOnly(float Amount)
{
    if (CurrentShield > 0.0f)
    {
        CurrentShield = FMath::Max(CurrentShield - Amount, 0.0f);
    }
}

void ALA_BaseCharacter::UpdateTeamTag(FGameplayTag NewTeamTag)
{

    FGameplayTag ParentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team"));

    // 현재 태그들 중 "Team" 하위 태그들만 따로 추출
    FGameplayTagContainer OldTeamTags = TeamTags.Filter(FGameplayTagContainer(ParentTeamTag));

    // 2. 찾아낸 기존 팀 태그들을 모두 제거
    TeamTags.RemoveTags(OldTeamTags);

    // 3. 새로운 팀 태그 추가
    TeamTags.AddTag(NewTeamTag);
}

void ALA_BaseCharacter::Decontaminate(float Amount)
{
    if (Contamination <= 0.0f)
        return;

    if (Amount <= 0.0f)
        return;

    Contamination = FMath::Clamp(Contamination - Amount, 0.0f, MaxContamination);
}

