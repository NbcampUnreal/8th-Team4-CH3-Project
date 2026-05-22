// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/LA_GameType.h"
#include "LA_ItemDataAsset.generated.h"

class UTexture2D;
class ULA_ItemEffect;
class USoundBase;

UCLASS(BlueprintType)
class LASTARTEMIS_API ULA_ItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()


public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
    // 아이템 구분용 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FName ItemID;

    // 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    // 아이템 설명
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText Description;

    // 아이템 2D 이미지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UTexture2D> ItemTexture2D;

    // 아이템 유형(무기, 소모성 아이템 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    ELA_ItemType ItemType = ELA_ItemType::None;

    // 슬롯 당 최대 개수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxStackCount = 99;

    // 소모성 아이템인지?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    bool bConsumable = false;

    // 체크 포인트에서 리필 가능한 최대 개수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxRefillCount;

    // 아이템 효과
    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Item")
    TObjectPtr<ULA_ItemEffect> Effect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<USoundBase> UseSound;
};
