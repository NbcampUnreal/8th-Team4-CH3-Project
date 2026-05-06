// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LA_PlayerWeaponHolderComponent.generated.h"

class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTARTEMIS_API ULA_PlayerWeaponHolderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULA_PlayerWeaponHolderComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // 컴포넌트를 소유하고있는 캐릭터
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1_Components")
    ACharacter* Character;

    // 조준 시점을 담당하는 카메라
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components")
    TObjectPtr<UCameraComponent> AimCamera;

    // 보유하고 있는 무기의 목록을 저장하는 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    TArray<AActor*> WeaponsContainer;

    // 장착중인 무기의 인덱스 번호
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    int32 EquipedIndex;

protected:
    // 발사 키 입력 상태를 나타내는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    bool bIsFired;

    // 연사가 가능한 무기 여부를 나타내는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    bool bIsAuto;

    // 총알을 발사할 수 있는지를 나타내는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "2_Weapon Settings")
    bool bCanFire;

    // 무기의 반동 수치를 나타내는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "2_Weapon Settings")
    float RecoilAmount;

    // 발사 모션의 애니메이션 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    TObjectPtr<UAnimMontage> FireMontage;

    // 재장전 모션의 애니메이션 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Weapon Settings")
    TObjectPtr<UAnimMontage> ReloadMontage;

public:
    /// <summary>
    /// 임의 무기에 대한 Mesh와 애니메이션 인스턴스를 할당하는 함수
    /// </summary>
    /// <param name="WeaponEquipedCharacterMesh">무기를 장착하고있는 캐릭터의 스켈레탈 메쉬</param>
    /// <param name="WeaponAnimInstance">장착한 무기에 맞춘 애니메이션 인스턴스</param>
    UFUNCTION(BlueprintCallable)
    void ActivateWeapon(AActor* WeaponActor, USkeletalMesh* WeaponEquipedCharacterMesh, UAnimInstance* WeaponAnimInstance);

    /// <summary>
    /// 현재 장착중인 무기에 대한 Mesh와 애니메이션 인스턴스를 제거(초기화)하는 함수
    /// </summary>
    UFUNCTION(BlueprintCallable)
    void DeactivateWeapon();

    /// <summary>
    /// 조준 시점을 담당하는 카메라 컴포넌트를 반환하는 함수
    /// </summary>
    /// <returns></returns>
    UFUNCTION(BlueprintCallable)
    FORCEINLINE UCameraComponent* GetAimCamera() const { return AimCamera; }

    /// <summary>
    /// 발사 키 입력 시작 시의 동작을 담당하는 함수
    /// </summary>
    UFUNCTION(BlueprintCallable)
    void StartFire();
    /// <summary>
    /// 발사 키 입력 종료 시의 동작을 담당하는 함수
    /// </summary>
    UFUNCTION(BlueprintCallable)
    void StopFire();

    /// <summary>
    /// 총알 발사, 반동 적용, 애니메이션 몽타주를 재생하는 함수
    /// </summary>
    UFUNCTION(BlueprintCallable)
    void Fire();

    /// <summary>
    /// 재장전 동작 함수
    /// </summary>
    UFUNCTION(BlueprintCallable)
    void Reload();

    // 총알을 다시 발사할 수 있도록 하는 함수
    UFUNCTION(BlueprintCallable)
    void NotifyCanFire();

protected:
    /// <summary>
    /// 조준 시점의 카메라를 무기를 장착한 캐릭터의 스켈레탈 메쉬에 부착하는 함수
    /// </summary>
    void AttachAimCamera();
};
