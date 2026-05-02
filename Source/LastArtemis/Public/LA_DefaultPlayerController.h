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

	// Change ViewPoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Input")
	TObjectPtr<UInputAction> ChangeViewpointInputAction;

#pragma endregion

};
