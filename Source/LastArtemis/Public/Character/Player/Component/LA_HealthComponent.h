// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LA_HealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, float);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShieldChangedDelegate, float, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContaminationDelegate, float, CurrentContamination, float, MaxContamination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTARTEMIS_API ULA_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULA_HealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
#pragma region Events

	// 체력 변화 이벤트
	FOnHealthChangedDelegate OnHealthChanged;

	// 실드량 변화 이벤트
	FOnShieldChangedDelegate OnShieldChanged;

    // 오염도 변화 이벤트
        UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnContaminationDelegate OnContaminationChanged;

	// 캐릭터 사망 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathDelegate OnDeath;

#pragma endregion

protected:
#pragma region By JoHyeonMuk

#pragma region Health

	// 캐릭터의 상태가 죽어있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;

	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

#pragma endregion

#pragma region Shield

	// 최대 실드량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxShield;

	// 현재 실드량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentShield;

#pragma endregion

#pragma region DefaultStatus

	// 기본 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackPower;

	// 기본 방어력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Defense;

#pragma endregion

#pragma region Contamination

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentContamination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxContamination;

#pragma endregion

public:
	/// <summary>
	/// 액터의 TakeDamage에서 호출하여 피해를 적용하는 함수
	/// 실제 적용된 데미지량 반환
	/// </summary>
	/// <param name="RawDamageAmount">가하는 데미지의 Raw 값(변화되지 않은 값)</param>
	/// <param name="bIsIgnoreShield">실드를 무시하고 데미지를 가하는지 여부(default = false)</param>
	/// <returns>실제로 적용된 데미지</returns>
	UFUNCTION(BlueprintCallable)
	float TakeDamage(float RawDamageAmount, bool bIsIgnoreShield = false);

    UFUNCTION(BlueprintCallable)
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable)
    float GetHealthPercent() const
    {
        if (MaxHealth <= 0.0f) return 0.0f;
        return CurrentHealth / MaxHealth;
    }
    UFUNCTION(BlueprintCallable)
    float GetCurrentHealth() const
    {
        return CurrentHealth;
    }
    UFUNCTION(BlueprintCallable)
    float GetMaxHealth() const
    {
        return MaxHealth;
    }
    UFUNCTION(BlueprintCallable)
    float GetCurrentShield() const
    {
        return CurrentShield;
    }

    UFUNCTION(BlueprintCallable)
    float GetMaxShield() const
    {
        return MaxShield;
    }

    UFUNCTION(BlueprintCallable)
    bool IsDead() const
    {
        return bIsDead;
    }

	void Die();

    // 오염도 관련 함수 추가

    UFUNCTION(BlueprintCallable)
    void AddContamination(float Amount);

    UFUNCTION(BlueprintCallable)
    void Decontaminate(float Amount);

    UFUNCTION(BlueprintCallable)
    float GetCurrentContamination() const
    {
        return CurrentContamination;
    }

    UFUNCTION(BlueprintCallable)
    float GetMaxContamination() const
    {
        return MaxContamination;
    }

    UFUNCTION(BlueprintCallable)
    float GetContaminationPercent() const
    {
        if (MaxContamination <= 0.0f) return 0.0f;
        return CurrentContamination / MaxContamination;
    }
};
