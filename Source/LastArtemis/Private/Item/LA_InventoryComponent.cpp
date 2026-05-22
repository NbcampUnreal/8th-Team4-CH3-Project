#include "Item/LA_InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Item/LA_ItemDataAsset.h"
#include "Item/LA_ItemEffect.h"
#include "Kismet/GameplayStatics.h"

ULA_InventoryComponent::ULA_InventoryComponent()
    :
    MaxSlotCount(5),
    MaxQuickSlotCount(2)
{
	PrimaryComponentTick.bCanEverTick = false;

    ItemSlots.SetNum(MaxSlotCount);
    QuickSlots.SetNum(MaxQuickSlotCount);
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

///////////////////////////
// 일반 슬롯
///////////////////////////

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

    // 런타임 중 MaxCount 증가할 경우
    if (ItemSlots.Num() < MaxSlotCount)
    {
        ItemSlots.SetNum(MaxSlotCount);
    }

    const FPrimaryAssetId ItemAssetId = ItemData->GetPrimaryAssetId();
    if (!ItemAssetId.IsValid())
    {
        return false;
    }

    int32 RemainingCount = AddCount;    // 남은 공간
    bool bAddedAny = false;             // 아이템 획득 성공 여부

    // 같은 아이템이 있으면 기존 슬롯부터 수량 증가
    for (FLA_ItemSlot& ItemSlot : ItemSlots)
    {
        // 다른 아이템일 경우
        if (!ItemSlot.IsSameItem(ItemAssetId))
        {
            continue;
        }

        // 꽉 찬 슬롯일 경우
        if (ItemSlot.CurrentCount >= ItemData->MaxStackCount)
        {
            continue;
        }

        // 남은 공간 계산
        const int32 RemainSpace = ItemData->MaxStackCount - ItemSlot.CurrentCount;
        const int32 AddToSlot = FMath::Min(RemainSpace, RemainingCount);

        if (AddToSlot <= 0)
        {
            continue;
        }

        ItemSlot.CurrentCount += AddToSlot;
        RemainingCount -= AddToSlot;
        bAddedAny = true;

        if (RemainingCount <= 0)
        {
            break;
        }
    }

    // 남은 수량은 빈 슬롯부터 채움
    if (RemainingCount > 0)
    {
        for (FLA_ItemSlot& ItemSlot : ItemSlots)
        {
            if (!ItemSlot.IsEmpty())
            {
                continue;
            }

            const int32 AddToSlot = FMath::Min(ItemData->MaxStackCount, RemainingCount);

            ItemSlot.ItemAssetId = ItemAssetId;
            ItemSlot.CurrentCount = AddToSlot;

            RemainingCount -= AddToSlot;
            bAddedAny = true;

            if (RemainingCount <= 0)
            {
                break;
            }
        }
    }

    // 아이템 획득 성공 및 소모성 아이템이면 퀵슬롯 자동 등록
    if (bAddedAny && ItemData->bConsumable)
    {
        AutoAssignToQuickSlot(ItemAssetId);
    }

    // 요청한 수량을 전부 넣었으면 true
    return RemainingCount <= 0;
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

    if (ItemData->UseSound)
    {
        UGameplayStatics::PlaySound2D(this, ItemData->UseSound);
    }

    // 효과 적용까지 성공하면 아이템 수량 감소
    return RemoveItem(SlotIndex, 1);
}

// 아이템 수량 감소
// 고정 슬롯 구조
// 수량이 0 이하면 해당 슬롯의 데이터만 비움
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

    // 수량이 0 이하면 해당 칸만 지움
    if (ItemSlot.CurrentCount <= 0)
    {
        ItemSlot.Clear();
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
            ItemSlot.HasItem() ? *ItemSlot.ItemAssetId.ToString() : TEXT("Empty"),
            ItemSlot.CurrentCount
        );
    }
}

///////////////////////////
// 퀵 슬롯
///////////////////////////

// 퀵 슬롯 아이템 등록
bool ULA_InventoryComponent::SetQuickItemSlot(int32 QuickSlotIndex, int32 ItemSlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex))
    {
        return false;
    }

    if (!ItemSlots.IsValidIndex(ItemSlotIndex))
    {
        return false;
    }

    const FLA_ItemSlot& ItemSlot = ItemSlots[ItemSlotIndex];
    if (!ItemSlot.HasItem())
    {
        return false;
    }

    ULA_ItemDataAsset* ItemData = GetLoadedItemData(ItemSlot.ItemAssetId);
    if (!ItemData)
    {
        return false;
    }

    if (!ItemData->bConsumable)
    {
        return false;
    }

    QuickSlots[QuickSlotIndex] = ItemSlot.ItemAssetId;

    return true;
}

FPrimaryAssetId ULA_InventoryComponent::GetQuickSlot(int32 QuickSlotIndex) const
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex))
        return FPrimaryAssetId();

    return QuickSlots[QuickSlotIndex];
}

// 퀵 슬롯 아이템 사용
bool ULA_InventoryComponent::UseQuickItem(int32 QuickSlotIndex, AActor* UseTarget)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex))
    {
        return false;
    }

    if (!UseTarget)
    {
        return false;
    }

    const FPrimaryAssetId ItemAssetId = QuickSlots[QuickSlotIndex];
    if (!ItemAssetId.IsValid())
    {
        return false;
    }

    return UseItemByAssetId(ItemAssetId, UseTarget);
}

// 퀵 슬롯 비우기
bool ULA_InventoryComponent::ClearQuickItemSlot(int32 QuickSlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex))
    {
        return false;
    }

    QuickSlots[QuickSlotIndex] = FPrimaryAssetId();

    return true;
}

void ULA_InventoryComponent::PrintQuickSlots() const
{
    if (QuickSlots.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickSlots is empty."));
        return;
    }

    for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
    {
        const FPrimaryAssetId& ItemAssetId = QuickSlots[Index];

        UE_LOG(LogTemp, Warning, TEXT("QuickSlot %d / Item: %s"),
            Index,
            ItemAssetId.IsValid() ? *ItemAssetId.ToString() : TEXT("Empty")
        );
    }
}

// 퀵 슬롯에서 아이템 ID를 찾아서 사용
bool ULA_InventoryComponent::UseItemByAssetId(const FPrimaryAssetId& ItemAssetId, AActor* UseTarget)
{
    if (!ItemAssetId.IsValid())
    {
        return false;
    }

    if (!UseTarget)
    {
        return false;
    }

    // 인벤토리에서 같은 ID를 가진 아이템의 슬롯을 찾아 사용
    for (int32 Index = 0; Index < ItemSlots.Num(); ++Index)
    {
        if (!ItemSlots[Index].IsSameItem(ItemAssetId))
        {
            continue;
        }

        return UseItem(Index, UseTarget);
    }

    // 인벤토리에 동일한 ID가 없을 시 false
    return false;
}

// 아이템 자동 등록
bool ULA_InventoryComponent::AutoAssignToQuickSlot(const FPrimaryAssetId& ItemAssetId)
{
    if (!ItemAssetId.IsValid())
    {
        return false;
    }

    if (QuickSlots.Num() <= 0)
    {
        return false;
    }

    // 이미 퀵 슬롯 등록되어 있는지 확인
    for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
    {
        if (QuickSlots[Index] == ItemAssetId)
        {
            return true;
        }
    }

    // 빈 슬롯 찾기
    for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
    {
        if (!QuickSlots[Index].IsValid())
        {
            QuickSlots[Index] = ItemAssetId;

            return true;
        }
    }

    // 퀵 슬롯 등록 자리 부족할 경우 false
    return false;
}

