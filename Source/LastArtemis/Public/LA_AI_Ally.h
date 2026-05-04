#pragma once

#include "CoreMinimal.h"
#include "LA_BaseCharacter.h"
#include "LA_AI_Ally.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_AI_Ally : public ALA_BaseCharacter
{
	GENERATED_BODY()
	
public:

	ALA_AI_Ally();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float HealAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float DeContaminateAmount;

	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void Heal();
	UFUNCTION(BlueprintCallable)
	void Decontaminate();
};
