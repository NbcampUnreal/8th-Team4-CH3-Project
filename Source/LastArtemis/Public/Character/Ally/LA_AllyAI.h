#pragma once

#include "CoreMinimal.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_AllyAI.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_AllyAI : public ALA_BaseCharacter
{
	GENERATED_BODY()

public:

    ALA_AllyAI();

    void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category= "Animation")
    UAnimMontage* DeathMontage;

    UFUNCTION(BlueprintCallable)
    void SetTargetEnemy(AActor* TargetEnemy);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

    void ResetHitState();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsHitAnimationPlaying = false;

    UFUNCTION()
    void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

};
