// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/LA_ItemDataAsset.h"
#include "LA_ItemSlot.generated.h"

// 소모성 아이템 슬롯
USTRUCT(BlueprintType)
struct FLA_ItemSlot
{
    GENERATED_BODY()

public:
    FLA_ItemSlot();

public:
    // 슬롯에 유효한 아이템이 있는지
    bool HasItem() const;
    // 슬롯이 비었는지
    bool IsEmpty() const;
    // 같은 아이템인지
    bool IsSameItem(const FPrimaryAssetId& OtherItemAssetId) const;
    // 특정 개수 이상 가지고 있는지
    bool HasEnoughCount(int32 RequiredCount) const;
    // 슬롯 초기화
    void Clear();


public:
    // 슬롯에 들어있는 아이템 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
    FPrimaryAssetId ItemAssetId;

    // 현재 아이템 보유 수량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Slot")
    int32 CurrentCount;
};
