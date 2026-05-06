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
	
	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	SkeletalMesh->SetupAttachment(Camera);
	SkeletalMesh->bCastDynamicShadow = false;
	SkeletalMesh->SetCastShadow(false);

	// Mesh의 Relative Transform 조정
	FRotator MeshRotator = FRotator(0, -90, 0);		// 회전
	FVector MeshLocation = FVector(-11, 0, -21);	// 위치
	FVector MeshScale = FVector(0.4, 0.4, 0.4);		// 크기
	SkeletalMesh->SetRelativeTransform(FTransform(MeshRotator, MeshLocation, MeshScale));

	// Rotation Axis setting
	bUseControllerRotationYaw = true;
	Camera->bUsePawnControlRotation = true;

	// 줌(조준) 시 사용될 카메라 컴포넌트 생성
	AimCamera = CreateDefaultSubobject<UCameraComponent>(FName("AimCamera"));
}

// Called when the game starts or when spawned
void ALA_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// InputMappingContext 적용
	if (Controller != nullptr)
	{
		if (APlayerController* playerController = Cast<APlayerController>(Controller))
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

	AimCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("AimEyeSocket"));
}

// Called every frame
void ALA_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAimed == true)
	{
		Camera->SetWorldRotation(GetControlRotation());
		FRotator AimCameraRotator = AimCamera->GetComponentRotation();
		AimCameraRotator.Roll = 0;
		AimCamera->SetWorldRotation(AimCameraRotator);
	}
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
		if (FireInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(FireInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::FireStartedAction);
			enhancedInputComponent->BindAction(FireInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::FireCompletedAction);
		}
		if (AimingInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(AimingInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::AimingStartedAction);
			enhancedInputComponent->BindAction(AimingInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::AimingCompletedAction);
		}
	}
}

void ALA_PlayerCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	UCameraComponent* component = bIsAimed == true ? AimCamera : Camera;

	if (component != nullptr)
	{
		component->GetCameraView(DeltaTime, DesiredView);
		return;
	}

	Super::CalcCamera(DeltaTime, DesiredView);
}

//void ALA_PlayerCharacter::AttachActorMeshes_Implementation(AActor* HoldActor, UMeshComponent* FirstPersonMesh, UMeshComponent* ThirdPersonMesh)
//{
//	// 캐릭터의 Mesh 교체
//	if (FirstPersonMesh != nullptr)
//	{
//		if (USkeletalMesh* SkeletalMesh1P = CastChecked<USkeletalMesh>(FirstPersonMesh))
//		{
//			// 캐릭터의 AnimInstance 초기화
//			if (GetMesh()->GetAnimInstance() != nullptr)
//			{
//				GetMesh()->ClearAnimScriptInstance();
//			}
//			// 캐릭터의 Mesh 변경
//			GetMesh()->SetSkeletalMesh(SkeletalMesh1P);
//		}
//	}
//	
//	// 캐릭터의 AnimInstance 교체
//}
//
//void ALA_PlayerCharacter::PlayAnimMontage_Implementation(UAnimMontage* FirstPersonMontage, UAnimMontage* ThirdPersonMontage)
//{
//	// 몽타주 재생
//	if (FirstPersonMontage != nullptr)
//	{
//		if (UAnimInstance* AnimInstance1P = GetMesh()->GetAnimInstance())
//		{
//			AnimInstance1P->Montage_Play(FirstPersonMontage);
//		}
//	}
//}
//
//void ALA_PlayerCharacter::UpdateHUDWidgetOnActor_Implementation(AActor* HoldActor)
//{
//}

FVector ALA_PlayerCharacter::GetFocusLocation()
{
	// LineTrace 실행
	FHitResult hitResult;
	FVector StartLocation = bIsAimed == true ? AimCamera->GetComponentLocation() : Camera->GetComponentLocation();
	FVector Direction = bIsAimed == true ? AimCamera->GetForwardVector() : Camera->GetForwardVector();
	FVector EndLocation = StartLocation + 10000 * Direction;	// 100 m 검사
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(hitResult, StartLocation, EndLocation, ECC_Visibility, queryParams);

	if (hitResult.bBlockingHit == true)
	{
		return hitResult.ImpactPoint;
	}
	return hitResult.TraceEnd;
}

//void ALA_PlayerCharacter::ActivateActor_Implementation(AActor* HoldActor)
//{ 
//	// 캐릭터의 Mesh 교체
//
//	// 캐릭터 Mesh의 AnimInstance 교체
//}
//
//void ALA_PlayerCharacter::DeactivateActor_Implementation(AActor* HoldActor)
//{
//	return;
//}

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
		// 앉아있는 상태가 아니면 걷기 상태로 변경
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

	// 토글 옵션이면서 앉아있는 상태의 경우
	if (CrouchInputMode == EMovementInputMode::Toggle && IsCrouched() == true)
	{
		// 앉기 상태 해제
		UnCrouch();
		return;
	}
	
	// 앉을 수 있는지 확인
	if (CanCrouch() == true)
	{
		// 달리기 상태인 경우 달리기 해제 (시야각 복원)
		if (bIsSprint == true)
		{
			SetSprintState(false);
		}

		// 앉기 실행
		Crouch();
	}
}

void ALA_PlayerCharacter::CrouchCompletedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// Hold 옵션이면서 앉아있는 상태의 경우
	if (CrouchInputMode == EMovementInputMode::Hold && IsCrouched() == true)
	{
		// 앉기 상태 해제
		UnCrouch();
		return;
	}
}

void ALA_PlayerCharacter::AimingStartedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// 토글 옵션이면서 조준 상태인 경우
	if (CrouchInputMode == EMovementInputMode::Toggle)
	{
		// 조준 상태 반전
		bIsAimed = !bIsAimed;
		return;
	}

	// 조준 상태로 설정
	bIsAimed = true;
}

void ALA_PlayerCharacter::AimingCompletedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// Hold 옵션이면서 앉아있는 상태의 경우
	if (CrouchInputMode == EMovementInputMode::Hold && bIsAimed == true)
	{
		// 비조준 상태로 설정
		bIsAimed = false;
		return;
	}
}

void ALA_PlayerCharacter::FireStartedAction()
{
	bIsFired = true;
	OnBeginShot();
}

void ALA_PlayerCharacter::FireCompletedAction()
{
	bIsFired = false;
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
	// 현재 상태와 동일한 경우 함수 조기 종료
	if (bIsSprint == bNewSprint)
	{
		return;
	}
	bIsSprint = bNewSprint;

	// 달리기 시작 시 앉은 상태 해제
	if (IsCrouched() == true)
	{
		UnCrouch();
	}

	// 달리기 상태로 변경
	if (bIsSprint == true)
	{
		// 이동 속도 변경 (27 km/s)
		GetCharacterMovement()->MaxWalkSpeed = 750;

		// 카메라의 FOV 값 변경
		Camera->FieldOfView = 103;
		return;
	}
	// 걷기 상태로 변경
	else
	{
		// 이동 속도 변경 (10.8 km/s)
		GetCharacterMovement()->MaxWalkSpeed = 300;

		// 카메라의 FOV 값 변경
		Camera->FieldOfView = 90;
		return;
	}
}

void ALA_PlayerCharacter::OnBeginShot()
{
	// 사격 입력이 들어오지 않고있거나 장전 중일 때 사격 불가
	if (bIsFired == false || bIsReloading == true)
	{
		return;
	}
	
	bIsReloading = true;
	FVector startLocation = GetMesh()->GetSocketLocation(FName("Muzzle"));
	FVector endLocation = GetFocusLocation();
	DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Red, false, 3.f, 0, 1);

	PlayAnimMontage(TestMontage, 1, FName("Defualt"));
}

void ALA_PlayerCharacter::OnEndShot()
{
	bIsReloading = false;
	if (bIsAuto == true)
	{
		OnBeginShot();
	}
}
