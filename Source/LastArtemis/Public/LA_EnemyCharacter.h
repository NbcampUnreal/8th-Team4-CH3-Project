#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "LA_BaseCharacter.h"
#include "LA_EnemyCharacter.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemyCharacter : public ALA_BaseCharacter, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	ALA_EnemyCharacter();

	virtual void TakeDamageCustom(float DamageAmount) override;
	virtual void Die() override;

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer GameplayTags;

};
