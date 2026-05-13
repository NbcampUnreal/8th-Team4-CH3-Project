#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "LA_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS()
class LASTARTEMIS_API ALA_BaseCharacter : public ACharacter
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float Contamination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxContamination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status | Tags")
    FGameplayTagContainer TeamTags;

public:
    ALA_BaseCharacter();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxShield;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentShield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Defense;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDead;


    // --- 기본 함수 ---
    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void TakeDamageCustom(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void Die();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer CharacterTags;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttributeChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttributeChangedSignature OnShieldChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeathSignature OnDeath;

    // 오염도 증가 함수
    void IncreaseContamination(float Amount);

    void PlayAttackMontage();

    // 실드만 깎는 특수 데미지 함수
    void ReduceShieldOnly(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetCurrentShield() const { return CurrentShield; }

    virtual void UpdateTeamTag(FGameplayTag NewTeamTag);

    // 오염도 제거 함수
    void Decontaminate(float Amount);
};
