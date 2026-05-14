// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LA_InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Item/LA_ItemDataAsset.h"
#include "Item/LA_ItemEffect.h"

// Sets default values for this component's properties
ULA_InventoryComponent::ULA_InventoryComponent()
    : MaxSlotCount(3)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// PrimaryAssetId를 이용해 Asset Manager에 있는 아이템 data Asset을 가져옴
// Project Setting -> Game -> AssetManager에서 Item 등록해야 됨
ULA_ItemDataAsset* ULA_InventoryComponent::GetLoadedItemData(const FPrimaryAssetId& ItemAssetId) const
{
    if (!ItemAssetId.IsValid())
    {
        return nullptr;
    }

    return Cast<ULA_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(ItemAssetId));
}

// 인벤토리에 아이템 추가
// 같은 아이템이 있으면 기존 슬롯부터 채우고,
// 공간이 부족할 시 새 슬롯에 남은 수량 추가
bool ULA_InventoryComponent::AddItem(ULA_ItemDataAsset* ItemData, int32 AddCount)
{
    if (!ItemData)
    {
        return false;
    }

    if (AddCount <= 0)
    {
        return false;
    }

    if (ItemData->MaxStackCount <= 0)
    {
        return false;
    }

    const FPrimaryAssetId ItemAssetId = ItemData->GetPrimaryAssetId();
    if (!ItemAssetId.IsValid())
    {
        return false;
    }

    // 같은 아이템이 있으면 기존 슬롯부터 수량 증가
    for (FLA_ItemSlot& ItemSlot : ItemSlots)
    {
        if (!ItemSlot.IsSameItem(ItemAssetId))
        {
            continue;
        }

        if (ItemSlot.CurrentCount >= ItemData->MaxStackCount)
        {
            continue;
        }

        int32 RemainSpace = ItemData->MaxStackCount - ItemSlot.CurrentCount;
        int32 AddToSlot = FMath::Min(RemainSpace, AddCount);

        ItemSlot.CurrentCount += AddToSlot;
        AddCount -= AddToSlot;

        // AddCount 모두 추가했으면 true 리턴
        if (AddCount <= 0)
        {
            return true;
        }
    }

    // AddCount 모두 추가 못했으면 새 슬롯 추가
    while (AddCount > 0)
    {
        if (ItemSlots.Num() >= MaxSlotCount)
        {
            return false;
        }

        FLA_ItemSlot NewSlot;
        NewSlot.ItemAssetId = ItemAssetId;
        NewSlot.CurrentCount = FMath::Min(ItemData->MaxStackCount, AddCount);

        ItemSlots.Add(NewSlot);

        AddCount -= NewSlot.CurrentCount;
    }

    return true;
}

// 아이템 효과 적용 및 수량 1개 감소
bool ULA_InventoryComponent::UseItem(int32 SlotIndex, AActor* UseTarget)
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    if (!UseTarget)
    {
        return false;
    }

    FLA_ItemSlot& ItemSlot = ItemSlots[SlotIndex];
    if (!ItemSlot.HasItem())
    {
        return false;
    }

    ULA_ItemDataAsset* ItemData = GetLoadedItemData(ItemSlot.ItemAssetId);
    if (!ItemData)
    {
        return false;
    }

    // 소모성 아이템만 사용 가능
    if (!ItemData->bConsumable)
    {
        return false;
    }

    if (!ItemData->Effect)
    {
        return false;
    }

    if (!ItemData->Effect->ApplyEffect(UseTarget))
    {
        return false;
    }

    // 효과 적용까지 성공하면 아이템 수량 감소
    return RemoveItem(SlotIndex, 1);
}

// 아이템 수량 감소
// 수량이 0 이하면 슬롯 자체를 제거
bool ULA_InventoryComponent::RemoveItem(int32 SlotIndex, int32 RemoveCount)
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    if (RemoveCount <= 0)
    {
        return false;
    }

    FLA_ItemSlot& ItemSlot = ItemSlots[SlotIndex];
    if (!ItemSlot.HasItem())
    {
        return false;
    }

    if (!ItemSlot.HasEnoughCount(RemoveCount))
    {
        return false;
    }

    ItemSlot.CurrentCount -= RemoveCount;

    if (ItemSlot.CurrentCount <= 0)
    {
        ItemSlots.RemoveAt(SlotIndex);
    }

    return true;
}

// 특정 아이템의 전체 수량
int32 ULA_InventoryComponent::GetItemTotalCount(ULA_ItemDataAsset* ItemData) const
{
    if (!ItemData)
    {
        return 0;
    }

    const FPrimaryAssetId ItemAssetId = ItemData->GetPrimaryAssetId();
    if (!ItemAssetId.IsValid())
    {
        return 0;
    }

    int32 TotalCount = 0;

    for (const FLA_ItemSlot& ItemSlot : ItemSlots)
    {
        // ID 같을 경우
        if (!ItemSlot.IsSameItem(ItemAssetId))
        {
            continue;
        }

        TotalCount += ItemSlot.CurrentCount;
    }

    return TotalCount;
}

// 아이템을 MaxRefillCount만큼 보급
// MaxRefiilCount 이상을 가지고 있으면 추가X
// 만약 CurrentCount = 2, MaxRefiilCount = 5 -> 3개 추가
bool ULA_InventoryComponent::RefillItem(ULA_ItemDataAsset* ItemData)
{
    if (!ItemData)
    {
        return false;
    }

    if (ItemData->MaxRefillCount <= 0)
    {
        return false;
    }

    int32 CurrentCount = GetItemTotalCount(ItemData);

    // 이미 MaxRefiilCount 이상이면 false
    if (CurrentCount >= ItemData->MaxRefillCount)
    {
        return false;
    }

    int32 AddCount = ItemData->MaxRefillCount - CurrentCount;

    return AddItem(ItemData, AddCount);
}

// 특정 슬롯 정보 복사해서 반환
bool ULA_InventoryComponent::GetItemSlot(int32 SlotIndex, FLA_ItemSlot& OutItemSlot) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    OutItemSlot = ItemSlots[SlotIndex];

    return true;
}

// 전체 슬롯 정보를 복사해서 반환
void ULA_InventoryComponent::GetAllItemSlots(TArray<FLA_ItemSlot>& OutItemSlots) const
{
    OutItemSlots = ItemSlots;
}

// 인벤토리 정보 출력
void ULA_InventoryComponent::PrintInventory() const
{
    if (ItemSlots.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory is empty."));
        return;
    }

    for (int32 Index = 0; Index < ItemSlots.Num(); ++Index)
    {
        const FLA_ItemSlot& ItemSlot = ItemSlots[Index];

        UE_LOG(LogTemp, Warning, TEXT("Slot %d / Item: %s / Count: %d"),
            Index,
            *ItemSlot.ItemAssetId.ToString(),
            ItemSlot.CurrentCount
        );
    }
}

