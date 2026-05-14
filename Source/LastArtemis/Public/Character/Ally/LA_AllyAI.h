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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float HealAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float DeContaminateAmount;


};
