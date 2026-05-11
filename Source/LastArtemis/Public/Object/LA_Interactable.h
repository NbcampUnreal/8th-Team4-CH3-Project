// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LA_Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULA_Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LASTARTEMIS_API ILA_Interactable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    /// <summary>
    /// ILA_Interactable을 상속받는 객체에 대하여 상호작용 하는 경우 호출되는 함수
    /// </summary>
    /// <param name="InteractInstigator">상호작용을 실시한 액터</param>
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable")
    void Interact(AActor* InteractInstigator);
    //virtual void Interact(AActor* InteractInstigator) = 0;
};
