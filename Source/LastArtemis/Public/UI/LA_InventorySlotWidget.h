// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;

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

    // 슬롯 인덱스
    int32 SlotIndex;

protected:
    // 아이템 아이콘 이미지
    UPROPERTY(meta = (BindWidget))
    UImage* Image_Icon;

    // 아이템 수량 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Quantity;

    // 현재 슬롯의 아이템 ID
    FPrimaryAssetId ItemAssetId;
};
