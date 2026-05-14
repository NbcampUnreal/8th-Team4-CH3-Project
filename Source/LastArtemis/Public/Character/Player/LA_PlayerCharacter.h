// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "LastArtemis/Character/LA_Holder.h"
#include "Components/TimelineComponent.h"
#include "LA_PlayerCharacter.generated.h"

// Forward Declaration
class UCameraComponent;

struct FInputActionValue;

// 키 입력 방식에 대한 옵션을 나타내는 열거형
// Toggle : 키 입력 시작 시 상태 변경
// Hold : 키 입력 중인 경우에만 상태 변경
UENUM()
enum class EMovementInputMode : uint8
{
	Toggle, Hold
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int32, int32);

// 속도가 변경되었을 경우 호출되는 이벤트의 타입
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeedChanged);

/*
* 
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

    // 걷기 속도 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float WalkSpeedFactor = 1;

    // 달리기 속도 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float SprintSpeedFactor = 1;

    // 앉은 상태의 속도 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
    float CrouchSpeedFactor = 1;

#pragma endregion

    // 이동 속도 속성이 변경되었을 경우 호출되는 이벤트
    UPROPERTY(BlueprintAssignable, Category = "LA_PlayerCharacter")
    FOnSpeedChanged OnMovementSpeedChanged;

protected:
    // 캐릭터 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer CharacterTags;

#pragma region Components

    // DeathCamera 위치를 관리하는 SpringArm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<class USpringArmComponent> SpringArmComponent;

    // 사망 시 캐릭터 주변을 회전하며 보여주는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> DeathCamera;

    // 캐릭터의 체력을 관리하는 컴포넌트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<class ULA_HealthComponent> HealthComponent;

    // 인벤토리 컴포넌트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
    TObjectPtr<class ULA_InventoryComponent> InventoryComponent;

#pragma endregion

#pragma region General Settings

	// 캐릭터의 현재 달리기 상태를 판별하는 변수 (true == 달리는 중)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "3_General Settings")
	bool bIsSprint = false;

    // 걷기 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3_General Settings")
    float WalkSpeed = 300;

    // 달리기 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3_General Settings")
    float SprintSpeed = 750;

    // 앉은 상태의 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3_General Settings")
    float CrouchSpeed = 150;

    // 블루프린트에서 설정할 일시정지 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

#pragma endregion

#pragma region Weapons Settings

    // 초기에 사용하는 무기 데이터
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "4_Weapon Settings")
    TObjectPtr<ULA_WeaponData> initialWeaponData;

    // 무기 퀵슬롯 (1, 2, 3)에 해당하는 OwnedWeapons의 Key 값을 관리하는 배열
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    TArray<FName> WeaponNameIndexer;

    // 보유한 무기 목록
    // { ULA_WeaponData::WeaponName, ULA_WeaponData* }
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    TMap<FName, ULA_WeaponData*> OwnedWeapons;
    // Key : 무기 데이터 클래스
    // Value : CDO와 비교하여 달라진 부분을 관리하는 구조체 또는 클래스
    //TMap<TSubclassOf<ULA_WeaponData>, FLA_WeaponDeltaProperty*> OwnedWeapons;
    
    // 현재 장착중인 무기를 저장하는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
    ALA_WeaponBase* EquipedWeapon;

#pragma endregion

#pragma region Death Event

    // 캐릭터 사망 시 시간에 따른 SpringArm의 TargetArmLength 값을 설정하는 커브 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "5_Death Event")
    TObjectPtr<UCurveFloat> TargetArmLengthCurve;

    // 캐릭터 사망 시 시간에 따른 SpringArm의 Rotation 값을 설정하는 커브 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "5_Death Event")
    TObjectPtr<UCurveVector> CameraBoomRotationEulerCurve;

    // 사망 이벤트 재생 시간
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "5_Death Event")
    float DeathCameraDuration;

    UPROPERTY()
    // 캐릭터 사망 시 재생되는 Timeline
    FTimeline DeathCameraTimeline;

#pragma endregion

public:
    // 화면에 보여지는 시점의 카메라를 반환하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    UCameraComponent* GetCameraComponent() const;

    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    FORCEINLINE void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const { TagContainer.AppendTags(CharacterTags); }

    // 장착하고 있는 무기를 반환하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    FORCEINLINE ALA_WeaponBase* GetEquipedWeapon() { return EquipedWeapon; }

    // 플레이어의 바라보는 방향으로 수행한 LineTrace 검사 결과를 반환하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    FHitResult LineTraceForward(float distance);

    // 보유하고 있는 다른 무기로 교체하는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    void SwapWeapon(int32 WeaponIndex);

    // 플레이어 캐릭터 사망 시 실행되는 함수
    UFUNCTION(BlueprintCallable, Category = "LA_PlayerCharacter")
    void OnPlayerDeath();

#pragma region Derived From IHolder

    /// <summary>
    /// 보유한 무기 목록에 임의 무기를 획득(추가)하는 함수
    /// 중복된 무기 획득 시 총알 보충
    /// </summary>
    /// <param name="WeaponData">획득한 무기 데이터</param>
    virtual void AddWeaponToPawn_Implementation(ULA_WeaponData* WeaponData) override;

    /// <summary>
    /// 임의 무기를 장착(부착)하는 함수
    /// </summary>
    /// <param name="Weapon">장착 하려는 무기 데이터</param>
    virtual void ActivateWeapon_Implementation(ULA_WeaponData* WeaponData) override;

    /// <summary>
    /// 무기를 장착 해제 또는 비활성화 하는 함수
    /// </summary>
    virtual void DeactivateWeapon_Implementation() override;

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

protected:
    // 액터의 이동 속도를 갱신하는 함수
    UFUNCTION()
    void UpdateMovementSpeed();

    // SpringArmComponent의 TargetArmLength 값을 조정하는 함수
    UFUNCTION()
    void UpdateCameraBoomTargetArmLength(float Value);

    // SpringArmComponent의 Rotation 값을 조정하는 함수
    UFUNCTION()
    void UpdateCameraBoomRotation(FVector Value);

#pragma region InputAction Binding

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

    // 상호작용 키 입력 시작 시 호출되는 함수
    void InteractStartedAction();

    // 일시정지 키에 입력 시 호출되는  함수
    void PauseAction();

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
	// (최대 속도 변경)
	void SetSprintState(bool bNewSprint);

    /// <summary>
    /// 캐릭터의 기본 SkeletalMesh Component를 비활성화 처리하는 함수
    /// </summary>
    void HideCharacterMesh();

    void SpawnWeaponActor();
};
