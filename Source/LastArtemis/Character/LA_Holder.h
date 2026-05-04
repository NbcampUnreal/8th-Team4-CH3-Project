// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LA_Holder.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULA_Holder : public UInterface
{
	GENERATED_BODY()
};

/**
 * 도구, 무기, 아이템 등 무언가를 손 또는 등과 같은 곳에 들 수 있는 패턴을 정의한 인터페이스
 * Ex) 캐릭터, 탈것 등
 */
class LASTARTEMIS_API ILA_Holder
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/// <summary>
	/// 임의 액터를 장착(부착)하는 함수
	/// </summary>
	/// <param name="HoldActor">부착되는 액터</param>
	/// <param name="FirstPersonMesh">1인칭 시점에서 부착되는 Mesh 컴포넌트</param>
	/// <param name="ThirdPersonMesh">3인칭 시점에서 부착되는 Mesh 컴포넌트</param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void AttachActorMeshes(AActor* HoldActor, UMeshComponent* FirstPersonMesh, UMeshComponent* ThirdPersonMesh);

	/// <summary>
	/// 부착되어있는 액터에 맞춘 애니메이션 몽타주를 재생하는 함수
	/// </summary>
	/// <param name="FirstPersonMontage">1인칭 시점의 애니메이션 몽타주</param>
	/// <param name="ThirdPersonMontage">3인칭 시점의 애니메이션 몽타주</param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void PlayAnimMontage(UAnimMontage* FirstPersonMontage, UAnimMontage* ThirdPersonMontage);

	/// <summary>
	/// 부착되어있는 액터의 정보를 HUD에 업데이트 하도록 호출하는 함수
	/// </summary>
	/// <param name="Actor"></param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void UpdateHUDWidgetOnActor(AActor* HoldActor);

	/// <summary>
	/// 카메라가 바라보고 있는 지점의 좌표를 얻는 함수
	/// </summary>
	/// <returns>바라보고 있는 지점의 좌표</returns>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	FVector GetFocusLocation();

	/// <summary>
	/// 기존에 보유하고 있으면서 비활성화 되어있는 액터를 활성화하는 함수
	/// 들고있는 액터 교체 시 사용됨
	/// </summary>
	/// <param name="HoldActor">활성화 시킬 액터</param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void ActivateActor(AActor* HoldActor);

	/// <summary>
	/// 기존에 보유하고 있으면서 활성화 되어있는 액터를 비활성화하는 함수
	/// </summary>
	/// <param name="HoldActor">비활성화 시킬 액터</param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void DeactivateActor(AActor* HoldActor);
};
