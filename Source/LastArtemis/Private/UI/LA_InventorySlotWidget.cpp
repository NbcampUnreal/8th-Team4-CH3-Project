// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LA_InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/LA_ItemDataAsset.h"
#include "Engine/AssetManager.h"

void ULA_InventorySlotWidget::UpdateSlotData(const FPrimaryAssetId& InItemAssetId, int32 InQuantity)
{
    ItemAssetId = InItemAssetId;

    if (ItemAssetId.IsValid())
    {
        // AssetManager를 통해 아이템 데이터 로드
        ULA_ItemDataAsset* ItemData = Cast<ULA_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(ItemAssetId));
        
        if (ItemData)
        {
            // 아이콘 설정
            if (Image_Icon && ItemData->ItemTexture2D)
            {
                Image_Icon->SetBrushFromTexture(ItemData->ItemTexture2D);
                Image_Icon->SetVisibility(ESlateVisibility::Visible);
            }

            // 수량 설정
            if (Text_Quantity)
            {
                Text_Quantity->SetText(FText::AsNumber(InQuantity));
                Text_Quantity->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
    else
    {
        // 비어있는 슬롯 처리
        //if (Image_Icon) Image_Icon->SetVisibility(ESlateVisibility::Hidden);
        if (Text_Quantity) Text_Quantity->SetVisibility(ESlateVisibility::Hidden);
    }
}

