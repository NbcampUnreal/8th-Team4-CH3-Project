// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_AllyAI.generated.h"

class UNiagaraSystem;

UCLASS()
class LASTARTEMIS_API ALA_AllyAI : public ALA_BaseCharacter
{
    GENERATED_BODY()

public:
    ALA_AllyAI();

    virtual void BeginPlay() override;

    // --- 애니메이션 및 상태 변수 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsFiring = false;

    // --- 시각 효과 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UNiagaraSystem* GunFireEffect;

    void PlayGunFireEffect();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetEnemy(AActor* TargetEnemy);

    // --- 전투 및 시스템 오버라이드 ---
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

    void ResetHitState();
    void ResetFiringState();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsHitAnimationPlaying = false;

    UFUNCTION()
    void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
