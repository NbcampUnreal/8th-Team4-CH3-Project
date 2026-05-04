// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LA_Holder.h"
#include "LA_PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

// 플레이어 캐릭터를 바라보는 시점을 나타내는 열거형
UENUM()
enum class ECharacterViewpoint : uint8
{
	FirstPerson UMETA(DisplayName = "First Person View"),
	ThirdPerson UMETA(DisplayName = "Third Person View")
};

// 달리기, 앉기 키 입력 방식에 대한 옵션을 나타내는 열거형
UENUM()
enum class EMovementInputMode : uint8
{
	Toggle, Hold
};

/*
* (Apex Legend) 스타일의 달리기 지정
*/
UCLASS()
class LASTARTEMIS_API ALA_PlayerCharacter : public ACharacter
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

	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& DesiredView) override;

public:

protected:
#pragma region Components

	// 캐릭터 기본 시야 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> Camera;

	// 줌(조준) 시 사용되는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> AimCamera;

#pragma endregion

#pragma region Input

	// InputMappingContext
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	// Move (W, A, S, D)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> MoveInputAction;

	// Jump (Space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> JumpInputAction;

	// Look (Mouse XY Axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> LookInputAction;
	
	// Sprint InputAction (Shift);
	// Triggers 옵션에서 Chorded Action을 추가하여 MoveInputAction이 활성화된 상태에서만 동작하도록 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> SprintInputAction;

	// Crouch InputAction (Control)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> CrouchInputAction;

	// Fire InputAction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> FireInputAction;

	// Zoom InputAction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> AimingInputAction;

#pragma endregion

#pragma region General Settings

	// 캐릭터의 현재 시점을 나타내는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3_General Settings")
	ECharacterViewpoint Viewpoint = ECharacterViewpoint::FirstPerson;

	// 달리기 키 입력에 대한 처리 방식을 결정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
	EMovementInputMode SprintInputMode = EMovementInputMode::Toggle;

	// 앉기 키 입력에 대한 처리 방식을 결정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
	EMovementInputMode CrouchInputMode = EMovementInputMode::Toggle;

	// 캐릭터의 현재 달리기 상태를 판별하는 변수 (true == 달리는 중)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3_General Settings")
	bool bIsSprint = false;

	// 줌(조준) 키 입력에 대한 처리 방식을 결정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3_General Settings")
	EMovementInputMode AimInputMode = EMovementInputMode::Toggle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bIsAimed;

#pragma endregion

	// 발사 입력 상태를 나타내는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
	bool bIsFired;

	// 들고있는 무기에 대하여 연사가 가능한지를 나타내는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4_Weapon Settings")
	bool bIsAuto;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "4_Weapon Settings")
	bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TObjectPtr<UAnimMontage> TestMontage;

public:
#pragma region Derived From IHolder

	///// <summary>
	///// 임의 액터를 장착(부착)하는 함수
	///// </summary>
	///// <param name="HoldActor">부착되는 액터</param>
	///// <param name="FirstPersonMesh">1인칭 시점에서 부착되는 Mesh 컴포넌트</param>
	///// <param name="ThirdPersonMesh">3인칭 시점에서 부착되는 Mesh 컴포넌트</param>
	//virtual void AttachActorMeshes_Implementation(AActor* HoldActor, UMeshComponent* FirstPersonMesh, UMeshComponent* ThirdPersonMesh) override;

	///// <summary>
	///// 부착되어있는 액터에 맞춘 애니메이션 몽타주를 재생하는 함수
	///// </summary>
	///// <param name="FirstPersonMontage">1인칭 시점의 애니메이션 몽타주</param>
	///// <param name="ThirdPersonMontage">3인칭 시점의 애니메이션 몽타주</param>
	//virtual void PlayAnimMontage_Implementation(UAnimMontage* FirstPersonMontage, UAnimMontage* ThirdPersonMontage) override;

	///// <summary>
	///// 부착되어있는 액터의 정보를 HUD에 업데이트 하도록 호출하는 함수
	///// </summary>
	///// <param name="Actor"></param>
	//virtual void UpdateHUDWidgetOnActor_Implementation(AActor* HoldActor) override;

	/// <summary>
	/// 카메라가 바라보고 있는 지점의 좌표를 얻는 함수
	/// </summary>
	/// <returns>바라보고 있는 지점의 좌표</returns>
	virtual FVector GetFocusLocation();

	///// <summary>
	///// 기존에 보유하고 있으면서 비활성화 되어있는 액터를 활성화하는 함수
	///// 들고있는 액터 교체 시 사용됨
	///// </summary>
	///// <param name="HoldActor">활성화 시킬 액터</param>
	//virtual void ActivateActor_Implementation(AActor* HoldActor) override;

	///// <summary>
	///// 기존에 보유하고 있으면서 활성화 되어있는 액터를 비활성화하는 함수
	///// </summary>
	///// <param name="HoldActor">비활성화 시킬 액터</param>
	//virtual void DeactivateActor_Implementation(AActor* HoldActor) override;

#pragma endregion

protected:
#pragma region InputAction BindingActions

	// 이동 움직임에 대한 InputAction에 연결되는 함수
	void MoveAction(const FInputActionValue& value);

	// 카메라 회전에 대한 InputAction에 적용되는 함수
	void LookAction(const FInputActionValue& value);
	
	// 달리기 키 입력 시작 시 호출되는 함수
	void SprintStartedAction();
	// 달리기 키 입력 중단 시 호출되는 함수
	void SprintCompletedAction();

	// 앉기 키 입력 시작 시 호출되는 함수
	void CrouchStartedAction();
	// 앉기 키 입력 중단 시 호출되는 함수
	void CrouchCompletedAction();

	// 조준 키 입력 시작 시 호출되는 함수
	void AimingStartedAction();
	// 조준 키 입력 종료 시 호출되는 함수
	void AimingCompletedAction();

	void FireStartedAction();
	void FireCompletedAction();

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

	UFUNCTION(BlueprintCallable, Category = "4_Weapon")
	void OnBeginShot();
	UFUNCTION(BlueprintCallable, Category = "4_Weapon")
	void OnEndShot();
};
