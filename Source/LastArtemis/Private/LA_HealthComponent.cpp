// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_HealthComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
ULA_HealthComponent::ULA_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	
	// 초기 체력은 100으로 설정
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;

	// 초기 실드량은 50으로 설정
	MaxShield = 50.0f;
	CurrentShield = MaxShield;

	// 기본 공격력 및 방어력 설정
	AttackPower = 10.0f;
	Defense = 5.0f;
}


// Called when the game starts
void ULA_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULA_HealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float ULA_HealthComponent::TakeDamage(float RawDamageAmount, bool bIsIgnoreShield)
{
	// 사망한 상태에서의 체력 감소 막기
	if (bIsDead == true)
	{
		return 0;
	}

	// 실제 데미지 계산 (방어력 적용)
	RawDamageAmount = FMath::Max(RawDamageAmount - Defense, 0.0f);


	// 실질 데미지가 존재하는 경우에만 실행
	if (RawDamageAmount > 0)
	{
		// 실제 적용된 데미지의 총합을 저장하는 변수 생성
		float ActualDamage = 0;

		// Modify Shield
		if (bIsIgnoreShield == true && CurrentShield > 0.0f)
		{
			// 실드에 적용되는 데미지 계산
			float DamageToShield = CurrentShield <= RawDamageAmount ? CurrentShield : RawDamageAmount;

			// 데미지에 따른 실드량 감소
			CurrentShield -= DamageToShield;

			// 적용된 데미지 누적
			ActualDamage += DamageToShield;

			// 남은 데미지 계산
			RawDamageAmount -= DamageToShield;

			// 실드량 변화 이벤트에 연결된 함수가 존재하는지 확인
			if (OnShieldChanged.IsBound())
			{
				// 변경 후 실드량을 매개변수로 이벤트 실행
				OnShieldChanged.Broadcast(CurrentShield);
			}
		}

		// Modify Health
		if (RawDamageAmount > 0)
		{
			// 실드에 적용되는 데미지 계산
			float DamageToHealth = CurrentHealth <= RawDamageAmount ? CurrentHealth : RawDamageAmount;

			// 체력 감소
			CurrentHealth -= DamageToHealth;

			// 적용된 데미지 누적
			ActualDamage += DamageToHealth;

			// 체력 변화 이벤트에 연결된 함수가 존재하는지 확인
			if (OnHealthChanged.IsBound())
			{
				// 변경 후 체력을 매개변수로 이벤트 실행
				OnHealthChanged.Broadcast(CurrentHealth);
			}

			// 현재 체력 확인
			if (CurrentHealth <= 0)
			{
				// 사망 함수 호출
				Die();
			}
		}

		// 실제 적용된 데미지 총합 반환
		return ActualDamage;
	}
	return 0;
}

void ULA_HealthComponent::Die()
{
	bIsDead = true;

	if (UCapsuleComponent* collision = GetOwner()->FindComponentByClass<UCapsuleComponent>())
	{
		collision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	if (OnDeath.IsBound())
	{
		OnDeath.Broadcast();
	}
}
