// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LA_DecontaminationItemEffect.h"
#include "Character/LA_BaseCharacter.h"

bool ULA_DecontaminationItemEffect::ApplyEffect(AActor* UseTarget)
{
    if (!UseTarget)
    {
        return false;
    }

    if (DecontaminationAmount <= 0.0f)
    {
        return false;
    }

    ALA_BaseCharacter* BaseCharacter = Cast<ALA_BaseCharacter>(UseTarget);
    if (!BaseCharacter)
    {
        return false;
    }

    BaseCharacter->Decontaminate(DecontaminationAmount);

    return true;
}
