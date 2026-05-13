// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LA_Holder.generated.h"

// 무기 기본 클래스 전방 선언
class ALA_WeaponBase;
class ULA_WeaponData;

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
    /// 보유한 무기 목록에 임의 무기를 획득(추가)하는 함수
    /// </summary>
    /// <param name="WeaponData">무기의 데이터</param>
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
    void AddWeaponToPawn(ULA_WeaponData* WeaponData);

	/// <summary>
	/// 임의 무기를 장착(부착)하는 함수
	/// </summary>
	/// <param name="Weapon">장착하려는 무기 데이터</param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void ActivateWeapon(ULA_WeaponData* WeaponData);

	/// <summary>
	/// 장착되어있는 임의 무기를 해제하는 함수
	/// </summary>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void DeactivateWeapon();

	/// <summary>
	/// 부착되어있는 무기의 정보를 HUD에 업데이트 하는 함수
	/// </summary>
	/// <param name="Actor"></param>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	void UpdateHUDWidgetOnActor(ALA_WeaponBase* HoldActor);

	/// <summary>
	/// 카메라가 바라보고 있는 지점의 좌표를 얻는 함수
	/// </summary>
	/// <returns>바라보고 있는 지점의 좌표</returns>
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ILA_Holder")
	FVector GetFocusLocation();
};
