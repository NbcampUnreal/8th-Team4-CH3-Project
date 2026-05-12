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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkeletalMesh");
    USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float HealAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float DeContaminateAmount;



    /*virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;*/

    virtual void TakeDamageCustom(float DamageAmount) override;
};
