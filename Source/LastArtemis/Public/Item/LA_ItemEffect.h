// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LA_ItemEffect.generated.h"

class AActor;

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class LASTARTEMIS_API ULA_ItemEffect : public UObject
{
	GENERATED_BODY()

public:
    // 아이템 효과 적용
    UFUNCTION(BlueprintCallable, Category = "Item Effect")
    virtual bool ApplyEffect(AActor* UseTarget);
};
