// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;

/**
 * 인벤토리의 개별 슬롯을 담당하는 위젯 클래스
 */
UCLASS()
class LASTARTEMIS_API ULA_InventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // 슬롯 데이터를 업데이트하는 함수
    void UpdateSlotData(const FPrimaryAssetId& InItemAssetId, int32 InQuantity);

    // 인벤토리 컴포넌트 설정
    void SetInventoryComponent(class ULA_InventoryComponent* InInventoryComp) { InventoryComponent = InInventoryComp; }

    // 아이템 사용 함수 (좌클릭 시 호출용)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseItemSlot();

    // 퀵슬롯 등록 함수 (우클릭 메뉴에서 호출용)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RegisterQuickSlot(int32 QuickSlotIndex);

    // 슬롯 인덱스
    int32 SlotIndex;

protected:
    // 아이템 아이콘 이미지
    UPROPERTY(meta = (BindWidget))
    UImage* Image_Icon;

    // 아이템 수량 텍스트를 감싸는 보더
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_Quantity;

    // 아이템 수량 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Quantity;

    // 현재 슬롯의 아이템 ID
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FPrimaryAssetId ItemAssetId;

    // 캐싱된 인벤토리 컴포넌트
    UPROPERTY()
    class ULA_InventoryComponent* InventoryComponent;
};
