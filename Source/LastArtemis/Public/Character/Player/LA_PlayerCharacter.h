// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "LastArtemis/Character/LA_Holder.h"
#include "LA_PlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

// 달리기, 앉기 키 입력 방식에 대한 옵션을 나타내는 열거형
UENUM()
enum class EMovementInputMode : uint8
{
	Toggle, Hold
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int32, int32);

/*
* (Apex Legend) 스타일의 달리기 지정
*/
UCLASS()
class LASTARTEMIS_API ALA_PlayerCharacter : public ACharacter, public ILA_Holder
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALA_PlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Get Actors Viewport
    virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

    FOnAmmoChangedSignature OnAmmoChangedSignature;

public:
#pragma region General Settings

    // 달리기 키 입력에 대한 처리 방식을 결정하는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    EMovementInputMode SprintInputMode = EMovementInputMode::Toggle;

    // 앉기 키 입력에 대한 처리 방식을 결정하는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    EMovementInputMode CrouchInputMode = EMovementInputMode::Toggle;

    // 조준 키 입력에 대한 처리 방식을 결정하는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    EMovementInputMode AimInputMode = EMovementInputMode::Toggle;

#pragma endregion

protected:
    // 캐릭터 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer CharacterTags;

	// 캐릭터 테스트 용도 카메라 사용 여부
	UPROPERTY(EditAnywhere, Category = "0_Debug", meta = (AllowPrivateAccess = true))
	bool bDebugCamera;

	// 캐릭터 테스트 용도 카메라
	UPROPERTY(EditAnywhere, Category = "0_Debug", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> TestCamera;

#pragma region Components

    TObjectPtr<class ULA_HealthComponent> HealthComponent;

#pragma endregion

#pragma region General Settings

	// 캐릭터의 현재 달리기 상태를 판별하는 변수 (true == 달리는 중)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "3_General Settings")
	bool bIsSprint = false;

    // 걷기 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float WalkSpeed = 300;

    // 달리기 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float SprintSpeed = 750;

    // 앉은 상태의 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float CrouchSpeed = 150;

#pragma endregion

#pragma region Weapons Settings

    // 초기에 사용하는 무기
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "4_Weapon Settings")
    TSubclassOf<ALA_WeaponBase> initialWeaponClass;

    // 무기 퀵슬롯 (1, 2, 3)에 해당하는 무기 클래스의 이름들을 관리하는 번호
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    TArray<FName> WeaponClassNameIndexer;

    // 보유한 무기 목록
    // { UClass::GetFName, ALA_WeaponBase* }
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    TMap<FName, ALA_WeaponBase*> OwnedWeapons;

    // 현재 장착중인 무기를 저장하는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    ALA_WeaponBase* EquipedWeapon;

#pragma endregion

public:
    // 화면에 보여지는 시점의 카메라를 반환하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    UCameraComponent* GetCameraComponent() const;

    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    FORCEINLINE void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const { TagContainer.AppendTags(CharacterTags); }


#pragma region Derived From IHolder

    ALA_WeaponBase* GetEquipedWeapon() { return EquipedWeapon; }

    /// <summary>
    /// 보유한 무기 목록에 임의 무기를 획득(추가)하는 함수
    /// 중복된 무기 획득 시 총알 보충
    /// </summary>
    /// <param name="WeaponClass">무기의 클래스</param>
    virtual void AddWeaponToPawn_Implementation(TSubclassOf<ALA_WeaponBase> WeaponClass) override;

    /// <summary>
    /// 임의 무기를 장착(부착)하는 함수
    /// </summary>
    /// <param name="Weapon">무기 액터</param>
    virtual void ActivateWeapon_Implementation(ALA_WeaponBase* Weapon) override;

    /// <summary>
    /// 임의 무기를 장착 해제 또는 비활성화 하는 함수
    /// </summary>
    /// <param name="Weapon">무기 액터</param>
    virtual void DeactivateWeapon_Implementation(ALA_WeaponBase* Weapon) override;

    /// <summary>
    /// 부착되어있는 무기의 정보를 HUD에 업데이트 하는 함수
    /// </summary>
    /// <param name="Actor"></param>
    virtual void UpdateHUDWidgetOnActor_Implementation(ALA_WeaponBase* HoldActor) override;

    /// <summary>
    /// 카메라가 바라보고 있는 지점의 좌표를 얻는 함수
    /// </summary>
    /// <returns>바라보고 있는 지점의 좌표</returns>
    virtual FVector GetFocusLocation_Implementation() override;

#pragma endregion

#pragma region Skill

    // 점멸 (순간이동)
    void Blink();

    // 무기 공격력 강화
    void EnhanceWeaponDamage(const float Duration);

    // 이동 속도 증가
    void EnhanceMovementSpeed(const float Duration);

#pragma endregion

    // 보유하고 있는 다른 무기로 교체하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    void SwapWeapon(int32 WeaponIndex);

    // 대량의 체력을 회복하는 함수 (999999)
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    void HealCharacter();

protected:
#pragma region InputAction BindingActions

	// 이동 움직임에 대한 InputAction에 연결되는 함수
	void MoveAction(const FInputActionValue& value);

	// 카메라 회전에 대한 InputAction에 적용되는 함수
	void LookAction(const FInputActionValue& value);
	
	// 달리기 키 입력 시작 시 호출되는 함수
	void SprintStartedAction();
    // 이동 입력 중단 시 호출되는 함수
    void MoveCompletedAction();
	// 달리기 키 입력 중단 시 호출되는 함수
	void SprintCompletedAction();

	// 앉기 키 입력 시작 시 호출되는 함수
	void CrouchStartedAction();
	// 앉기 키 입력 중단 시 호출되는 함수
	void CrouchCompletedAction();

    // 공격 키 입력 시작 시 호출되는 함수
	void FireStartedAction();
    // 공격 키 입력 종료 시 호출되는 함수
	void FireCompletedAction();

	// 조준 키 입력 시작 시 호출되는 함수
	void AimingStartedAction();
	// 조준 키 입력 종료 시 호출되는 함수
	void AimingCompletedAction();

    // 재장전 키 입력 시작 시 호출되는 함수
    void ReloadStartedAction();

#pragma endregion

	/// <summary>
	/// 데미지를 받았을 경우 해당 데미지를 처리하는 로직을 정의하는 함수
	/// </summary>
	/// <param name="Damage">데미지</param>
	/// <param name="DamageEvent"></param>
	/// <param name="EventInstigator"></param>
	/// <param name="DamageCauser">데미지의 원인이 되는 액터</param>
	/// <returns>실제로 적용된 데미지</returns>
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// 캐릭터의 걷기/달리기 상태를 변경하는 함수
	// (최대 속도 변경 + 카메라 시야각 변경)
	void SetSprintState(bool bNewSprint);

    /// <summary>
    /// 캐릭터의 SkeletalMesh Component를 비활성화 처리하는 함수
    /// </summary>
    void HideCharacterMesh();

};
