// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "LA_BaseCharacter.generated.h"

// 🎯 1. 전방 선언을 통해 컴포넌트 클래스가 존재함을 알립니다.
class ULA_HealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS()
class LASTARTEMIS_API ALA_BaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ALA_BaseCharacter();

    // --- 전투 및 대미지 시스템 (오버라이드) ---
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    // --- 애니메이션 및 사운드 에셋 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    class USoundBase* HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AttackMontage;

    // --- 태그 시스템 변수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer CharacterTags;

    // 💡 블루프린트에서 기존 UI 바인딩용 딜리게이트들을 계속 쓸 수 있도록 이벤트 선언은 그대로 유지합니다.
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttributeChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttributeChangedSignature OnShieldChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeathSignature OnDeath;

    // --- 핵심 공통 함수들 ---
    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void TakeDamageCustom(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void Die();

    void IncreaseContamination(float Amount);
    void Decontaminate(float Amount);
    void PlayAttackMontage();
    void ReduceShieldOnly(float Amount);
    virtual void UpdateTeamTag(FGameplayTag NewTeamTag);

    // 🎯 2. 자식 클래스(보스, 플레이어, 터렛)나 외부에서 컴포넌트에 접근할 수 있도록 Getter 제공
    FORCEINLINE ULA_HealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
    virtual void BeginPlay() override;

    // 🎯 3. [가장 중요] 모든 캐릭터가 물려받아 사용할 실시간 헬스 컴포넌트 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    ULA_HealthComponent* HealthComponent;

    // --- 오염도 및 기존 상태 변수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float Contamination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxContamination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status | Tags")
    FGameplayTagContainer TeamTags;

    // 본체 사망 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDead;

    // 💡 [수술 부위] MaxHealth, CurrentHealth, MaxShield, CurrentShield, AttackPower, Defense는
    // 이제 전부 현묵님이 만드신 'HealthComponent' 내부 변수로 관리하므로, 이 헤더에서는 중복을 막기 위해 과감히 지웠습니다!

private:
    // 컴포넌트 내부 델리게이트와 연동하여 블루프린트 이벤트를 릴레이해줄 내부 콜백 함수
    UFUNCTION()
    void HandleHealthChanged(float CurrentHP, float MaxHP);

    UFUNCTION()
    void HandleShieldChanged(float CurrentShield, float MaxShield);
};
