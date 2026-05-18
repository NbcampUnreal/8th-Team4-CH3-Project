// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/LA_ItemSlot.h"
#include "LA_InventoryComponent.generated.h"

class ULA_ItemDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTARTEMIS_API ULA_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULA_InventoryComponent();

protected:
    // AssetManager에 있는 아이템 Data Asset을 Id로 가져옴
    ULA_ItemDataAsset* GetLoadedItemData(const FPrimaryAssetId& ItemAssetId) const;


public:
    // 아이템 추가
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(ULA_ItemDataAsset* ItemData, int32 AddCount = 1);
    // 아이템 사용
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(int32 SlotIndex, AActor* UseTarget);
    // 아이템 수량 감소
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(int32 SlotIndex, int32 RemoveCount = 1);

    // 아이템 전체 개수 반환
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemTotalCount(ULA_ItemDataAsset* ItemData) const;
    // 체크 포인트에서 아이템 보급
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RefillItem(ULA_ItemDataAsset* ItemData);

    // 특정 아이템 슬롯 정보
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetItemSlot(int32 SlotIndex, FLA_ItemSlot& OutItemSlot) const;
    // 전체 인벤토리 슬롯 정보
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void GetAllItemSlots(TArray<FLA_ItemSlot>& OutItemSlots) const;

    // 인벤토리 내 아이템 출력
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrintInventory() const;

    // 퀵 슬롯 등록 함수
    UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
    bool SetQuickItemSlot(int32 QuickSlotIndex, int32 ItemSlotIndex);
    // 퀵 슬롯 아이템 사용
    UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
    bool UseQuickItem(int32 QuickSlotIndex, AActor* UseTarget);
    // 퀵슬롯 아이템 초기화
    UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
    bool ClearQuickItemSlot(int32 QuickSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
    void PrintQuickSlots() const;

protected:
    bool UseItemByAssetId(const FPrimaryAssetId& ItemAssetId, AActor* UseTarget);
    // 아이템 획득 시 퀵슬롯 빈칸에 자동 등록
    bool AutoAssignToQuickSlot(const FPrimaryAssetId& ItemAssetId);

protected:
    ///////////////////////////
    // 일반 슬롯
    ///////////////////////////
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxSlotCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FLA_ItemSlot> ItemSlots;

    ///////////////////////////
    // 퀵 슬롯
    ///////////////////////////
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|QuickSlot")
    int32 MaxQuickSlotCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|QuickSlot")
    TArray<FPrimaryAssetId> QuickSlots;
};
