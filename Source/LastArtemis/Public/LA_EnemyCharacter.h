#pragma once

#include "CoreMinimal.h"

#include "LA_BaseCharacter.h"
#include "LA_EnemyCharacter.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemyCharacter : public ALA_BaseCharacter
{
	GENERATED_BODY()

public:
	ALA_EnemyCharacter();
	
	virtual void TakeDamageCustom(float DamageAmount) override;
	virtual void Die() override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;
	
};
