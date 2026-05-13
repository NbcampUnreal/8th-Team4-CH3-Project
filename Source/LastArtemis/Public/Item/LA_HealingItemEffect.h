// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/LA_ItemEffect.h"
#include "LA_HealingItemEffect.generated.h"

class AActor;

UCLASS(Blueprintable)
class LASTARTEMIS_API ULA_HealingItemEffect : public ULA_ItemEffect
{
	GENERATED_BODY()

public:
    // 체력 회복
    virtual bool ApplyEffect(AActor* UseTarget) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Healing")
    float RecoveryAmount = 100.0f;
};
