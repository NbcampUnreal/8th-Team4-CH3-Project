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

    void BeginPlay() override;

    // 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category= "Animation")
    UAnimMontage* DeathMontage;
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsFiring = false;

    // 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UNiagaraSystem* GunFireEffect;

    void PlayGunFireEffect();



    UFUNCTION(BlueprintCallable)
    void SetTargetEnemy(AActor* TargetEnemy);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

    void ResetHitState();
    // 타이머가 끝났을 때 사격 상태를 풀어주는 함수
    void ResetFiringState();


protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsHitAnimationPlaying = false;

    UFUNCTION()
    void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

};
