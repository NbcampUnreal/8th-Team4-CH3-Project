// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LA_ItemSlot.h"

FLA_ItemSlot::FLA_ItemSlot()
    :
    ItemAssetId(FPrimaryAssetId()),
    CurrentCount(0)
{
}

bool FLA_ItemSlot::HasItem() const
{
    return ItemAssetId.IsValid() && CurrentCount > 0;
}

bool FLA_ItemSlot::IsEmpty() const
{
    return !HasItem();
}

bool FLA_ItemSlot::IsSameItem(const FPrimaryAssetId& OtherItemAssetId) const
{
    return HasItem() && ItemAssetId == OtherItemAssetId;
}

bool FLA_ItemSlot::HasEnoughCount(int32 RequiredCount) const
{
    if (RequiredCount <= 0)
    {
        return false;
    }

    return HasItem() && CurrentCount >= RequiredCount;
}

void FLA_ItemSlot::Clear()
{
    ItemAssetId = FPrimaryAssetId();
    CurrentCount = 0;
}
