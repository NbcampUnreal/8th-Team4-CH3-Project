// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LA_InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Item/LA_InventoryComponent.h"
#include "UI/LA_InventorySlotWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/LA_GameModeBase.h"

void ULA_InventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Close)
    {
        Button_Close->OnClicked.AddDynamic(this, &ULA_InventoryWidget::CloseInventory);
    }
}

void ULA_InventoryWidget::InitializeInventory(ULA_InventoryComponent* InInventoryComp)
{
    InventoryComponent = InInventoryComp;
    RefreshInventory();
}

void ULA_InventoryWidget::RefreshInventory()
{
    if (!InventoryComponent || !GridPanel_Inventory || !SlotWidgetClass) return;

    // 기존 슬롯 제거
    GridPanel_Inventory->ClearChildren();

    TArray<FLA_ItemSlot> ItemSlots;
    InventoryComponent->GetAllItemSlots(ItemSlots);

    int32 ColumnCount = 5; // 한 줄에 표시할 슬롯 수

    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        ULA_InventorySlotWidget* SlotWidget = CreateWidget<ULA_InventorySlotWidget>(this, SlotWidgetClass);
        if (SlotWidget)
        {
            SlotWidget->SlotIndex = i;
            SlotWidget->UpdateSlotData(ItemSlots[i].ItemAssetId, ItemSlots[i].CurrentCount);

            int32 Row = i / ColumnCount;
            int32 Column = i % ColumnCount;

            GridPanel_Inventory->AddChildToUniformGrid(SlotWidget, Row, Column);
        }
    }
}

void ULA_InventoryWidget::CloseInventory()
{
    // 게임 일시정지 해제
    if (ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->ResumeGame();
    }

    // 입력 모드 전환
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    // 위젯 제거
    RemoveFromParent();
}

