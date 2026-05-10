#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "LA_BaseCharacter.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedDelegate, float, NewHealth);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldChangedDelegate, float, NewShield);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

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

    /*
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class ALA_Weapon> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    class ALA_Weapon* CurrentWeapon; */

    // --- 기본 함수 ---
    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void TakeDamageCustom(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    virtual void Die();

    /*UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void EquipWeapon(TSubclassOf<ALA_Weapon> WeaponClass);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    virtual float GetAttackPower() const; */

    //UPROPERTY(BlueprintAssignable, Category = "Events")
    //FOnHealthChangedDelegate OnHealthChanged;

    //UPROPERTY(BlueprintAssignable, Category = "Events")
    //FOnShieldChangedDelegate OnShieldChanged;

    //UPROPERTY(BlueprintAssignable, Category = "Events")
    //FOnDeathDelegate OnDeath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer CharacterTags;

    // 오염도 증가 함수
    void IncreaseContamination(float Amount);

    void PlayAttackMontage();

    // 쉴드만 깎는 특수 데미지 함수
    void ReduceShieldOnly(float Amount);

    FORCEINLINE float GetCurrentShield() const { return CurrentShield; }

    virtual void UpdateTeamTag(FGameplayTag NewTeamTag);


};
