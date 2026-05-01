// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

public:

protected:
#pragma region Components

	// 1인칭 시점 캐릭터 메쉬 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;

	// 1인칭 시점을 보여주는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	// 3인칭 시점 카메라가 붙어있는 SpringArm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	// 3인칭 시점을 보여주는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1_Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> ThirdPersonCameraComponent;

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

	// Change ViewPoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2_Input")
	TObjectPtr<UInputAction> ChangeViewpointInputAction;

#pragma endregion


private:
	// 캐릭터의 현재 시점을 나타내는 변수
	UPROPERTY(EditInstanceOnly, Category = "Viewpoint")
	ECharacterViewpoint Viewpoint = ECharacterViewpoint::FirstPerson;

public:
	// 1인칭 시점의 SkeletalMeshComponent를 반환하는 함수
	UFUNCTION(BlueprintCallable, Category = "1_Components")
	FORCEINLINE USkeletalMeshComponent* Get1PMesh() const { return FirstPersonMeshComponent; }

	// 3인칭 시점의 SkeletalMeshComponent를 반환하는 함수
	UFUNCTION(BlueprintCallable, Category = "1_Components")
	FORCEINLINE USkeletalMeshComponent* Get3PMesh() const { return GetMesh(); }

	// 시점을 설정하는 함수
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCharacterViewPoint(ECharacterViewpoint newViewpoint);
	
	// 시점 변경에 대한 InputAction에 적용되는 함수
	void ChangeViewpointAction(const FInputActionValue& value);

protected:
	// 이동 움직임에 대한 InputAction에 연결되는 함수
	void MoveAction(const FInputActionValue& value);
	// 카메라 회전에 대한 InputAction에 적용되는 함수
	void LookAction(const FInputActionValue& value);

	/// <summary>
	/// 데미지를 받았을 경우 해당 데미지를 처리하는 로직을 정의하는 함수
	/// </summary>
	/// <param name="Damage">데미지</param>
	/// <param name="DamageEvent"></param>
	/// <param name="EventInstigator"></param>
	/// <param name="DamageCauser">데미지의 원인이 되는 액터</param>
	/// <returns>실제로 적용된 데미지</returns>
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
