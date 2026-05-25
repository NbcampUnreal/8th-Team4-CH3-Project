// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LA_InventoryWidget.generated.h"

class UUniformGridPanel;
class UButton;
class ULA_InventoryComponent;
class ULA_InventorySlotWidget;

/**
 * 인벤토리 전체 화면을 담당하는 위젯 클래스
 */
UCLASS()
class LASTARTEMIS_API ULA_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // 인벤토리 초기화 및 데이터 연결
    void InitializeInventory(ULA_InventoryComponent* InInventoryComp);

    // 인벤토리 리스트 갱신
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    // 인벤토리 닫기
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CloseInventory();

protected:
    // 슬롯들이 배치될 그리드 패널
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* GridPanel_Inventory;

    // 닫기 버튼
    UPROPERTY(meta = (BindWidget))
    UButton* Button_Close;

    // 개별 슬롯을 생성할 때 사용할 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<ULA_InventorySlotWidget> SlotWidgetClass;

    // 캐싱된 인벤토리 컴포넌트
    UPROPERTY()
    ULA_InventoryComponent* InventoryComponent;
};
