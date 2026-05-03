// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_PlayerCharacter.h"
#include "Camera/CameraComponent.h"		// UCameraComponent
#include "GameFramework/SpringArmComponent.h"	// USpringArmComponent
#include "GameFramework/CharacterMovementComponent.h"	// UCharacterMovementComponent
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"		// UEnhancedInputComponent, FInputActionValue
#include "LA_DefaultPlayerController.h"		// ALS_DefaultPlayerController
#include "LA_HealthComponent.h"

// Sets default values
ALA_PlayerCharacter::ALA_PlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// FirstPersonMeshComponent Settings
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMeshComponent"));
	FirstPersonMeshComponent->SetupAttachment(GetMesh());
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	// FirstPersonCameraComponent Settings
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent);
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(0, 10, 90 + BaseEyeHeight), FRotator(0, 90, 0));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// SpringArmComponent Settings
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
	SpringArmComponent->TargetArmLength = 400;
	SpringArmComponent->bUsePawnControlRotation = true;

	// ThirdPersonCameraComponent Settings
	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("ThirdPersonCameraComponent"));
	ThirdPersonCameraComponent->SetupAttachment(SpringArmComponent);

	// Extra Settings
	USkeletalMeshComponent* thirdPersonMeshComponent = GetMesh();
	thirdPersonMeshComponent->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	thirdPersonMeshComponent->SetOwnerNoSee(true);

	// 초기 시점은 1인칭 시점으로 설정
	Viewpoint = ECharacterViewpoint::FirstPerson;

	// Rotation Axis setting
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// 이동 방향으로 캐릭터 회전 옵션 비활성화
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

// Called when the game starts or when spawned
void ALA_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// InputMappingContext 적용
	if (Controller != nullptr)
	{
		if (APlayerController* playerController = CastChecked<APlayerController>(Controller))
		{
			if (ULocalPlayer* localPlayer = playerController->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* subSystem = localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					if (MappingContext != nullptr)
					{
						subSystem->AddMappingContext(MappingContext, 0);
					}
				}
			}
		}
	}

	// 캐릭터 시점 설정 적용
	SetCharacterViewPoint(Viewpoint);
}

// Called every frame
void ALA_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALA_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

#pragma region Ver Custom PlayerController

	//// 특정 플레이어 컨트롤러로 기존 플레이어 컨트롤러 변환
	//if (ALA_DefaultPlayerController* playerController = CastChecked<ALA_DefaultPlayerController>(Controller))
	//{
	//	// 플레이어의 입력 컴포넌트를 향상된 입력 컴포넌트로 변환
	//	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	//	{
	//		// 작성한 플레이어 컨트롤러에서 설정한 InputAction 에셋에 함수 연결

	//		// move
	//		if (playerController->MoveInputAction != nullptr)
	//		{
	//			enhancedInputComponent->BindAction(playerController->MoveInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::MoveAction);
	//		}
	//		// jump
	//		if (playerController->JumpInputAction != nullptr)
	//		{
	//			enhancedInputComponent->BindAction(playerController->JumpInputAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	//			enhancedInputComponent->BindAction(playerController->JumpInputAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	//		}
	//		// look
	//		if (playerController->LookInputAction != nullptr)
	//		{
	//			enhancedInputComponent->BindAction(playerController->LookInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::LookAction);
	//		}
	//		// change viewpoint
	//		if (playerController->ChangeViewpointInputAction != nullptr)
	//		{
	//			enhancedInputComponent->BindAction(playerController->ChangeViewpointInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::ChangeViewpointAction);
	//		}
	//	}
	//}

#pragma endregion

	// 플레이어의 입력 컴포넌트를 향상된 입력 컴포넌트로 변환
	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// move
		if (MoveInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::MoveAction);
		}
		// jump
		if (JumpInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			enhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		// look
		if (LookInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::LookAction);
		}
		// change viewpoint
		if (ChangeViewpointInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(ChangeViewpointInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::ChangeViewpointAction);
		}
		// sprint
		if (SprintInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::SprintStartedAction);
			enhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::SprintCompletedAction);
		}
		// crouch
		if (CrouchInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::CrouchStartedAction);
			enhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::CrouchCompletedAction);
		}
	}
}

void ALA_PlayerCharacter::SetCharacterViewPoint(ECharacterViewpoint NewViewpoint)
{
	// 기존 시점과 동일하게 설정하는 경우 함수 조기 종료
	if (Viewpoint == NewViewpoint)
	{
		return;
	}
	Viewpoint = NewViewpoint;

	// 1인칭 설정
	if (Viewpoint == ECharacterViewpoint::FirstPerson)
	{
		// 컨트롤러의 회전 값을 액터의 현재 회전 값으로 적용
		Controller->SetControlRotation(GetActorRotation());

		// 1인칭 시점 카메라 활성화
		FirstPersonCameraComponent->SetActive(true);
		// 3인칭 시점 카메라 비활성화
		ThirdPersonCameraComponent->SetActive(false);

		// 컨트롤러의 Yaw 회전 입력이 액터에 적용되도록 설정
		bUseControllerRotationYaw = true;

		// 이동 방향으로 캐릭터 회전 옵션 비활성화
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// SkeletalMeshComponent 설정
		GetMesh()->SetOwnerNoSee(true);		// 3인칭 캐릭터가 보여지지 않도록 설정
		FirstPersonMeshComponent->SetOnlyOwnerSee(true);	// 1인칭 캐릭터가 보여지도록 설정
		return;
	}
	// 3인칭 설정
	if (Viewpoint == ECharacterViewpoint::ThirdPerson)
	{
		// 3인칭 시점 카메라 활성화
		ThirdPersonCameraComponent->SetActive(true);
		// 1인칭 시점 카메라 비활성화
		FirstPersonCameraComponent->SetActive(false);

		// 컨트롤러의 Yaw 회전 입력이 액터에 적용되지 않도록 설정
		bUseControllerRotationYaw = false;

		// 이동 방향으로 캐릭터 회전 옵션 활성화
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// SkeletalMeshComponent 설정
		GetMesh()->SetOwnerNoSee(false);		// 3인칭 캐릭터가 보여지도록 설정
		FirstPersonMeshComponent->SetOnlyOwnerSee(false);	// 1인칭 캐릭터가 보여지지 않도록 설정
	}
}

void ALA_PlayerCharacter::MoveAction(const FInputActionValue& value)
{
	// 컨트롤러 확인
	if (Controller == nullptr)
	{
		return;
	}

	// 입력 값을 FVector 타입으로 얻기
	FVector InputVector = value.Get<FVector>();
	if (InputVector.IsNearlyZero() == false)
	{
		// 컨트롤러의 회전값 얻기
		FRotator ControlRotation = GetControlRotation();

		// 컨트롤러 회전을 기준으로 방향 조정
		FVector Direction = InputVector.RotateAngleAxis(ControlRotation.Yaw, FVector::UpVector);

		// 이동 입력 대입
		AddMovementInput(Direction);
		return;
	}
}

void ALA_PlayerCharacter::LookAction(const FInputActionValue& value)
{
	// 컨트롤러 확인
	if (Controller == nullptr)
	{
		return;
	}

	// 입력 값을 FVector 타입으로 얻기
	FVector InputVector = value.Get<FVector>();
	if (InputVector.IsNearlyZero() == false)
	{
		// Yaw, Pitch 회전값 적용
		AddControllerYawInput(InputVector.X);
		AddControllerPitchInput(InputVector.Y);
	}
}

void ALA_PlayerCharacter::ChangeViewpointAction(const FInputActionValue& value)
{
	// 컨트롤러 확인
	if (Controller == nullptr)
	{
		return;
	}

	// 현재 시점과 다른 시점으로 변경
	if (Viewpoint == ECharacterViewpoint::FirstPerson)
	{
		SetCharacterViewPoint(ECharacterViewpoint::ThirdPerson);
	}
	else
	{
		SetCharacterViewPoint(ECharacterViewpoint::FirstPerson);
	}
}

void ALA_PlayerCharacter::SprintStartedAction()
{
	if (Controller == nullptr)
	{
		return;
	}
	
	// 현재 걷기 상태에서 키 입력 시 달리기
	// 현재 달리기 상태에서 키 입력 시 걷기
	if (SprintInputMode == EMovementInputMode::Toggle)
	{
		// 현재 달리기 상태의 반대 값 적용
		SetSprintState(!bIsSprint);
		return;
	}

	// 현재 상태와 관계없이 즉시 달리기 상태로 변경
	if (SprintInputMode == EMovementInputMode::Hold)
	{
		SetSprintState(true);
		return;
	}
}

void ALA_PlayerCharacter::SprintCompletedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// Hold 옵션인 경우에만 동작
	if (SprintInputMode == EMovementInputMode::Hold)
	{
		// 웅크린 상태가 아니면 걷기 상태로 변경
		if (IsCrouched() == false)
		{
			SetSprintState(false);
		}
	}
}

void ALA_PlayerCharacter::CrouchStartedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// 토글 옵션인 경우
	if (CrouchInputMode == EMovementInputMode::Toggle && IsCrouched() == true)
	{
		// 앉기 상태 해제
		UnCrouch();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("일어서기"));
		return;
	}
	
	// 앉을 수 있는지 확인
	if (CanCrouch() == true)
	{
		// 달리기 상태인 경우 달리기 해제
		if (bIsSprint == true)
		{
			SetSprintState(false);
		}

		// 앉기 실행
		Crouch();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("웅크리기"));
	}
}

void ALA_PlayerCharacter::CrouchCompletedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// Hold 옵션인 경우
	if (CrouchInputMode == EMovementInputMode::Hold && IsCrouched() == true)
	{
		// 앉기 상태 해제
		UnCrouch();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("일어서기"));
		return;
	}
}

float ALA_PlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (ULA_HealthComponent* HealthComponent = FindComponentByClass<ULA_HealthComponent>())
	{
		float ActualDamage = HealthComponent->TakeDamage(Damage, false);
		
		return ActualDamage;
	}
	return Damage;
}

void ALA_PlayerCharacter::SetSprintState(bool bNewSprint)
{
	if (bIsSprint == bNewSprint)
	{
		return;
	}
	bIsSprint = bNewSprint;

	// 웅크린 상태인 경우 웅크리기 해제
	if (IsCrouched() == true)
	{
		UnCrouch();
	}

	if (bIsSprint == true)
	{
		// 이동 속도 변경 (27 km/s)
		GetCharacterMovement()->MaxWalkSpeed = 750;

		// 카메라의 FOV 값 변경
		FirstPersonCameraComponent->FieldOfView = 103;
		ThirdPersonCameraComponent->FieldOfView = 103;

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("달리기 시작"));
		return;
	}
	else
	{
		// 이동 속도 변경 (5.4 km/s)
		GetCharacterMovement()->MaxWalkSpeed = 150;

		// 카메라의 FOV 값 변경
		FirstPersonCameraComponent->FieldOfView = 90;
		ThirdPersonCameraComponent->FieldOfView = 90;

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("달리기 종료"));
		return;
	}
}
