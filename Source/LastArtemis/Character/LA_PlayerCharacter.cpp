// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_PlayerCharacter.h"
#include "Camera/CameraComponent.h"		// UCameraComponent
#include "GameFramework/CharacterMovementComponent.h"	// UCharacterMovementComponent
#include "EnhancedInputSubsystems.h"    // UInputMappingContext, UInputAction
#include "EnhancedInputComponent.h"		// UEnhancedInputComponent, FInputActionValue
#include "Character/LA_DefaultPlayerController.h"		// ALS_DefaultPlayerController
#include "Character/Player/Component/LA_HealthComponent.h"

// Sets default values
ALA_PlayerCharacter::ALA_PlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 기본 이동 속도 변경 (10.8 km/s)
    GetCharacterMovement()->MaxWalkSpeed = 300;

    // 테스트 용도 카메라 활성화
    bDebugCamera = true;

    // 테스트 용도 카메라 생성
	TestCamera = CreateDefaultSubobject<UCameraComponent>(FName("TestCamera"));
	TestCamera->SetupAttachment(RootComponent);
	TestCamera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));

	// Rotation setting
	bUseControllerRotationYaw = true;
	TestCamera->bUsePawnControlRotation = true;

    // 캐릭터의 기본 Mesh 비활성화
    HideCharacterMesh();
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

    // 캐릭터의 기본 USkeletalMeshComponent 숨기기
    HideCharacterMesh();
}

// Called every frame
void ALA_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // 테스트 카메라 사용 여부에 따른 카메라 컴포넌트 제어
    if (TestCamera->IsActive() != bDebugCamera)
    {
        bDebugCamera == true ? TestCamera->Deactivate() : TestCamera->Activate();
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
	if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// move
		if (MoveInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::MoveAction);
			enhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::MoveCompletedAction);
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
        // Fire
		if (FireInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(FireInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::FireStartedAction);
			enhancedInputComponent->BindAction(FireInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::FireCompletedAction);
		}
        // Aim
		if (AimInputAction != nullptr)
		{
			enhancedInputComponent->BindAction(AimInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::AimingStartedAction);
			enhancedInputComponent->BindAction(AimInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::AimingCompletedAction);
		}
	}
}

void ALA_PlayerCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    Super::CalcCamera(DeltaTime, OutResult);

    // 장착중인 무기가 존재하지 않으면 테스트 카메라의 View 사용
    if (EquipedWeapon == nullptr)
    {
        return TestCamera->GetCameraView(DeltaTime, OutResult);
    }

    // 장착중인 무기의 카메라 컴포넌트 사용

    return;
}

#pragma region Derived From ILA_Holder

void ALA_PlayerCharacter::ActivateWeapon_Implementation(ALA_WeaponBase* WeaponActor, USkeletalMesh* WeaponCharacterMesh)
{
    // 장착하려는 무기와 중복되는 인덱스 찾기
    int32 DuplicatedIndex = WeaponsContainer.IndexOfByPredicate([&](const AActor* Element)
        {
            return Element == WeaponActor;
        });

    // 현재 장착중인 무기와 중복되는 경우
    if (EquipedWeapon == WeaponActor)
    {
        // 함수 조기 종료
        return;
    }

    // 보유한 무기 목록에서 동일한 무기 검색
    if (WeaponsContainer.Contains(WeaponActor) == true)
    {
        // 장착중인 무기 교체
        EquipedWeapon = WeaponActor;

        // 무기가 보이도록 설정
        EquipedWeapon->SetActorHiddenInGame(false);
    }
    else
    {
        WeaponsContainer.Add(WeaponActor);
    }

    // 무기 할당 전 기존 무기 해제
    DeactivateWeapon();

    // 무기 액터 부착
    WeaponActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

    return;
}

void ALA_PlayerCharacter::DeactivateWeapon_Implementation()
{
    if (EquipedWeapon == nullptr)
    {
        return;
    }

    // 총을 발사중인 경우 발사 중지
    EquipedWeapon->StopFire();

    // 장착중인 무기 숨기기
    EquipedWeapon->SetActorHiddenInGame(true);
    return;
}

void ALA_PlayerCharacter::UpdateHUDWidgetOnActor_Implementation(ALA_WeaponBase* HoldActor)
{
}

FVector ALA_PlayerCharacter::GetFocusLocation_Implementation()
{

    // LineTrace 실행
    FHitResult HitResult;
    FVector StartLocation = TestCamera->GetComponentLocation();
    FVector Direction = TestCamera->GetForwardVector();
    if (EquipedWeapon != nullptr)
    {
        StartLocation = EquipedWeapon->GetActorLocation();
        Direction = EquipedWeapon->GetActorForwardVector();
    }
    else
    {
        // 장착중인 무기가 존재하지 않으면 오류 메시지 출력
        UE_LOG(LogTemp, Warning, TEXT("Try to get focus Location with invalid Weapon"));
        return FVector::ZeroVector;
    }
    FVector EndLocation = StartLocation + 10000 * Direction;	// 100 m 검사
    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(this);

    GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, queryParams);

    if (HitResult.bBlockingHit == true)
    {
        return HitResult.ImpactPoint;
    }
    return HitResult.TraceEnd;
}

#pragma endregion

#pragma region Input Action

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

void ALA_PlayerCharacter::MoveCompletedAction()
{
    if (Controller == nullptr)
    {
        return;
    }

    // Toggle 옵션인 경우
    if (SprintInputMode == EMovementInputMode::Toggle)
    {
        // 앉아있는 상태가 아니면 걷기 상태로 변경
        if (IsCrouched() == false)
        {
            SetSprintState(false);
        }
    }
}

void ALA_PlayerCharacter::SprintCompletedAction()
{
	if (Controller == nullptr)
	{
		return;
	}

	// Hold 옵션인 경우
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
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to start Fire"));
        return;
    }

    EquipedWeapon->StartFire();
}

void ALA_PlayerCharacter::FireCompletedAction()
{
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to stop Fire"));
        return;
    }

    EquipedWeapon->StopFire();
}

#pragma endregion

float ALA_PlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (ULA_HealthComponent* HealthComponent = FindComponentByClass<ULA_HealthComponent>())
	{
		float ActualDamage = HealthComponent->TakeDamage(Damage, false);

		return ActualDamage;
	}
    UE_LOG(LogTemp, Warning, TEXT("No Health Component or Member Variables"));
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

    // 앉은 상태 해제
    if (IsCrouched() == true)
    {
        UnCrouch();
    }

    // 달리기 상태로 변경
    if (bIsSprint == true)
    {
        // 이동 속도 변경 (27 km/s)
        GetCharacterMovement()->MaxWalkSpeed = 750;
        return;
    }
    // 걷기 상태로 변경
    else
    {
        // 이동 속도 변경 (10.8 km/s)
        GetCharacterMovement()->MaxWalkSpeed = 300;
        return;
    }
}

void ALA_PlayerCharacter::HideCharacterMesh()
{
    if (USkeletalMeshComponent* CharacterMesh = GetMesh())
    {
        // Mesh 삭제
        CharacterMesh->SetSkeletalMesh(nullptr);

        // 렌더링되지 않도록 설정
        CharacterMesh->SetVisibility(false, true);
        CharacterMesh->SetHiddenInGame(true, true);
        CharacterMesh->SetCastShadow(false);

        // Collision 제거
        CharacterMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

        // Overlap Event 방지
        CharacterMesh->SetGenerateOverlapEvents(false);

        // 컴포턴트 업데이트 막기
        CharacterMesh->SetComponentTickEnabled(false);
    }
}

#pragma region Legacy

void ALA_PlayerCharacter::PlayWeaponAnimMontage_Implementation(UAnimMontage* Montage)
{
}

void ALA_PlayerCharacter::OnBeginShot()
{
	//// 사격 입력이 들어오지 않고있거나 장전 중일 때 사격 불가
	//if (bIsFired == false || bIsReloading == true)
	//{
	//	return;
	//}
	//
	//bIsReloading = true;
	//FVector startLocation = GetMesh()->GetSocketLocation(FName("Muzzle"));
	//FVector endLocation = GetFocusLocation();
	//DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Red, false, 3.f, 0, 1);

	//PlayAnimMontage(TestMontage, 1, FName("Defualt"));
}

void ALA_PlayerCharacter::OnEndShot()
{
 //   // 장전 완료 상태로 변경
	//bIsReloading = false;

 //   // 연사가 가능한 경우 현재 발사 키가 입력중인지 확인
	//if (bIsAuto == true && bIsFired == true)
	//{
 //       // 총알 발사 함수 실행
	//	OnBeginShot();
	//}
}

#pragma endregion
