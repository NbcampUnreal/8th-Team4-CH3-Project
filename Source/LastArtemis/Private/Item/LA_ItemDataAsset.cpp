// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LA_ItemDataAsset.h"

FPrimaryAssetId ULA_ItemDataAsset::GetPrimaryAssetId() const
{
    // ItemID가 비어있으면 에셋 이름 사용, Item ID가 있으면 ItemID 사용
    const FName DataAssetName = ItemID.IsNone() ? GetFName() : ItemID;

    return FPrimaryAssetId(TEXT("Item"), DataAssetName);
}
