// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LA_DefaultPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 *
 */
UCLASS()
class LASTARTEMIS_API ALA_DefaultPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

public:
#pragma region Enhanced Input

    // InputMappingContext
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputMappingContext> MappingContext;

    // Move (W, A, S, D)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> MoveInputAction;

    // Jump (Space)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> JumpInputAction;

    // Look (Mouse XY Axis)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> LookInputAction;

    // Sprint InputAction (Shift);
    // Triggers 옵션에서 Chorded Action을 추가하여 MoveInputAction이 활성화된 상태에서만 동작하도록 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> SprintInputAction;

    // Crouch InputAction (Control)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> CrouchInputAction;

    // Fire InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> FireInputAction;

    // Zoom InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> AimInputAction;

    // Reload InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> ReloadInputAction;

    // Skill A InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> SkillAInputAction;

    // Skill B InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> SkillBInputAction;

    // No.1 Weapon QuickSlot InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> WeaponSlot1InputAction;

    // No.2 Weapon QuickSlot InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> WeaponSlot2InputAction;

    // No.3 Weapon QuickSlot InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> WeaponSlot3InputAction;

    // No.3 Weapon QuickSlot InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> InteractInputAction;

    // CommandTarget InputAction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> CommandTargetAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
    TObjectPtr<UInputAction> PauseInputAction;
#pragma endregion
};
