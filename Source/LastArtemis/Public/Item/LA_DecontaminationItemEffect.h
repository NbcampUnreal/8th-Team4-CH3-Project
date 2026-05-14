// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/LA_ItemEffect.h"
#include "LA_DecontaminationItemEffect.generated.h"

class AActor;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class LASTARTEMIS_API ULA_DecontaminationItemEffect : public ULA_ItemEffect
{
	GENERATED_BODY()

public:
    // 오염도 제거
    virtual bool ApplyEffect(AActor* UseTarget) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decontamination")
    float DecontaminationAmount = 100.0f;
};
