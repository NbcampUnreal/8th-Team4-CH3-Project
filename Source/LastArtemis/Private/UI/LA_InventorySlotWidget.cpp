// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LA_InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/LA_ItemDataAsset.h"
#include "Engine/AssetManager.h"
#include "Item/LA_InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

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
        if (Image_Icon) Image_Icon->SetVisibility(ESlateVisibility::Hidden);
        if (Text_Quantity) Text_Quantity->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ULA_InventorySlotWidget::UseItemSlot()
{
    if (InventoryComponent && ItemAssetId.IsValid())
    {
        // 플레이어 캐릭터를 타겟으로 아이템 사용
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (InventoryComponent->UseItem(SlotIndex, PlayerPawn))
        {
            // 사용 성공 시 데이터 갱신 (보통 InventoryComponent에서 델리게이트 등으로 위젯 전체를 갱신하겠지만, 필요하다면 여기서 추가 처리 가능)
        }
    }
}

void ULA_InventorySlotWidget::RegisterQuickSlot(int32 QuickSlotIndex)
{
    if (InventoryComponent && ItemAssetId.IsValid())
    {
        InventoryComponent->SetQuickItemSlot(QuickSlotIndex, SlotIndex);
    }
}

