// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LA_HealingItemEffect.h"
#include "Character/Player/Component/LA_HealthComponent.h"

bool ULA_HealingItemEffect::ApplyEffect(AActor* UseTarget)
{
    if (!UseTarget)
    {
        return false;
    }

    if (RecoveryAmount <= 0.0f)
    {
        return false;
    }

    ULA_HealthComponent* HealthComponent = UseTarget->FindComponentByClass<ULA_HealthComponent>();
    if (!HealthComponent)
    {
        return false;
    }

    HealthComponent->Heal(RecoveryAmount);

    return true;
}
