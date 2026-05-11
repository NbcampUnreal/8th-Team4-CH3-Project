// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/LA_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"	        // UCharacterMovementComponent
#include "GameFramework/SpringArmComponent.h"                   // USpringArmComponent
#include "Components/CapsuleComponent.h"                        // UCapsuleComponent
#include "Camera/CameraComponent.h"		                        // UCameraComponent
#include "EnhancedInputComponent.h"		                        // UEnhancedInputComponent, FInputActionValue
#include "Character/LA_DefaultPlayerController.h"		        // ALA_DefaultPlayerController
#include "Character/Player/Component/LA_HealthComponent.h"      // ULA_HealthComponent
#include "LastArtemis/Weapon/LA_WeaponBase.h"                   // ALA_WeaponBase

// Sets default values
ALA_PlayerCharacter::ALA_PlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 체력 관련 컴포넌트 부착
    HealthComponent = CreateDefaultSubobject<ULA_HealthComponent>(FName("HealthComponent"));

    // 캐릭터 기본 이동 속도 변경 (10.8 km/s)
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // SpringArm 컴포넌트 생성
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
    SpringArmComponent->TargetArmLength = 0;

    // 사망 시점 카메라 생성
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(FName("DeathCamera"));
	DeathCamera->SetupAttachment(SpringArmComponent);

	// Rotation setting
	bUseControllerRotationYaw = true;

    // 캐릭터의 기본 Mesh 비활성화
    HideCharacterMesh();

    // GameplayTag 추가
    FGameplayTag PlayerTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));
    if (PlayerTag.IsValid())
    {
        CharacterTags.AddTag(PlayerTag);
    }
}

// Called when the game starts or when spawned
void ALA_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

    // 캐릭터의 기본 USkeletalMeshComponent 숨기기
    HideCharacterMesh();

    // 기본 무기 획득
    if (IsValid(initialWeaponClass) == true)
    {
        // 초기 무기 획득
        ILA_Holder::Execute_AddWeaponToPawn(this, initialWeaponClass);

        if (WeaponClassNameIndexer.IsEmpty() == false)
        {
            // 0번 인덱스의 무기 장착
            SwapWeapon(0);
        }
    }
}

// Called every frame
void ALA_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    DeathCameraTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void ALA_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 플레이어 컨트롤러 변환
	if (ALA_DefaultPlayerController* LA_Controller = Cast<ALA_DefaultPlayerController>(Controller))
	{
		// 플레이어의 입력 컴포넌트를 향상된 입력 컴포넌트로 변환
		if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			// move
			if (LA_Controller->MoveInputAction != nullptr)
			{
				enhancedInputComponent->BindAction(LA_Controller->MoveInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::MoveAction);
				enhancedInputComponent->BindAction(LA_Controller->MoveInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::MoveCompletedAction);
			}
			// jump
			if (LA_Controller->JumpInputAction != nullptr)
			{
				enhancedInputComponent->BindAction(LA_Controller->JumpInputAction, ETriggerEvent::Started, this, &ACharacter::Jump);
				enhancedInputComponent->BindAction(LA_Controller->JumpInputAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			}
			// look
			if (LA_Controller->LookInputAction != nullptr)
			{
				enhancedInputComponent->BindAction(LA_Controller->LookInputAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::LookAction);
			}
            // sprint
            if (LA_Controller->SprintInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->SprintInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::SprintStartedAction);
                enhancedInputComponent->BindAction(LA_Controller->SprintInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::SprintCompletedAction);
            }
            // crouch
            if (LA_Controller->CrouchInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->CrouchInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::CrouchStartedAction);
                enhancedInputComponent->BindAction(LA_Controller->CrouchInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::CrouchCompletedAction);
            }
            // fire
            if (LA_Controller->FireInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->FireInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::FireStartedAction);
                enhancedInputComponent->BindAction(LA_Controller->FireInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::FireCompletedAction);
            }
            // aim
            if (LA_Controller->AimInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->AimInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::AimingStartedAction);
                enhancedInputComponent->BindAction(LA_Controller->AimInputAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::AimingCompletedAction);
            }
            // reload
            if (LA_Controller->ReloadInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->ReloadInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::ReloadStartedAction);
            }
            // skill A (Blink)
            if (LA_Controller->SkillAInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->SkillAInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::Blink);
            }
            // skill B (Enhance Speed)
            if (LA_Controller->SkillBInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->SkillBInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::EnhanceMovementSpeed, 5.f);
            }
            // Weapon QuickSlot 1
            if (LA_Controller->WeaponSlot1InputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->WeaponSlot1InputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::SwapWeapon, 0);
            }
            // Weapon QuickSlot 2
            if (LA_Controller->WeaponSlot2InputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->WeaponSlot2InputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::SwapWeapon, 1);
            }
            // Weapon QuickSlot 3
            if (LA_Controller->WeaponSlot3InputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->WeaponSlot3InputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::SwapWeapon, 2);
            }
		}
	}

}

void ALA_PlayerCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    Super::CalcCamera(DeltaTime, OutResult);

    UCameraComponent* Camera = GetCameraComponent();

    return Camera->GetCameraView(DeltaTime, OutResult);
}

UCameraComponent* ALA_PlayerCharacter::GetCameraComponent() const
{
    return EquipedWeapon == nullptr ? DeathCamera.Get() : EquipedWeapon->GetFirstPersonCamera();
}

#pragma region Derived From ILA_Holder

void ALA_PlayerCharacter::AddWeaponToPawn_Implementation(TSubclassOf<ALA_WeaponBase> WeaponClass)
{
    if (IsValid(WeaponClass) == false)
    {
        return;
    }

    // 획득한 무기 클래스의 고유 값 얻기
    FName CLS_UID = WeaponClass->GetFName();

    // 중복 무기 획득 검사
    if (OwnedWeapons.Contains(CLS_UID) == true)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("이미 소유하고 있는 무기입니다. - %s"), *CLS_UID.ToString()));
        ALA_WeaponBase* Weapon = OwnedWeapons[CLS_UID];

        // 무기의 총알 채워넣기
        //Weapon->
        return;
    }
    else
    {
        // 무기 인스턴스 생성
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;   // 소유자 설정
        SpawnParams.Instigator = this;
        ALA_WeaponBase* Weapon = GetWorld()->SpawnActor<ALA_WeaponBase>(WeaponClass, GetActorTransform(), SpawnParams);

        // 보유한 무기 목록에 추가
        OwnedWeapons.Add({ CLS_UID, Weapon });
        WeaponClassNameIndexer.Add(CLS_UID);

        // 캐릭터에 무기 부착
        Weapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Weapon->SetActorRelativeLocation(FVector(0, 0, BaseEyeHeight));

        // 획득한 무기 비활성화 처리
        ILA_Holder::Execute_DeactivateWeapon(this, Weapon);
    }
}

void ALA_PlayerCharacter::ActivateWeapon_Implementation(ALA_WeaponBase* Weapon)
{
    if (IsValid(Weapon) == false)
    {
        return;
    }

    FString DebugMessage = FString::Printf(TEXT("무기(%s) 장착"), *Weapon->GetName());
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, DebugMessage);

    // 현재 장착 중인 무기를 활성화하는 경우
    if (Weapon == EquipedWeapon)
    {
        // 함수 조기 종료
        return;
    }

    // 다른 무기 활성화 시 현재 무기 비활성화
    if (EquipedWeapon != nullptr)
    {
        ILA_Holder::Execute_DeactivateWeapon(this, EquipedWeapon);
    }

    // 선택 무기 활성화
    Weapon->SetActorHiddenInGame(false);

    // 장착 무기 교체
    EquipedWeapon = Weapon;

    ILA_Holder::Execute_UpdateHUDWidgetOnActor(this, EquipedWeapon);

    return;
}

void ALA_PlayerCharacter::DeactivateWeapon_Implementation(ALA_WeaponBase* Weapon)
{
    if (Weapon == nullptr)
    {
        return;
    }

    // 무기 조준 상태 해제 및 발사 중지
    Weapon->bIsAiming = false;
    Weapon->StopFire();

    // 무기 숨기기
    Weapon->SetActorHiddenInGame(true);

    // 무기의 Collision 비활성화
    Weapon->SetActorEnableCollision(false);
    return;
}

void ALA_PlayerCharacter::UpdateHUDWidgetOnActor_Implementation(ALA_WeaponBase* HoldActor)
{
    // HoldActor가 nullptr일 경우를 대비해 안전하게 EquipedWeapon을 사용
    ALA_WeaponBase* TargetWeapon = HoldActor ? HoldActor : EquipedWeapon;

    if (TargetWeapon)
    {
        OnAmmoChangedSignature.Broadcast(
            TargetWeapon->GetCurrentMagazineAmmo(),
            TargetWeapon->GetMaxMagazineSize()
        );
    }
}

FVector ALA_PlayerCharacter::GetFocusLocation_Implementation()
{
    UCameraComponent* Camera = GetCameraComponent();

    // LineTrace 실행
    FHitResult HitResult;
    FVector StartLocation = Camera->GetComponentLocation();
    FVector Direction = Camera->GetForwardVector();
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

#pragma region Skill

void ALA_PlayerCharacter::Blink()
{
    // 카메라 얻기
    UCameraComponent* Camera = GetCameraComponent();

    // 바라보는 지점을 향하여 LineTrace 실행
    FHitResult HitResult;
    FVector StartLocation = Camera->GetComponentLocation();
    FVector Direction = Camera->GetForwardVector();
    FVector EndLocation = StartLocation + 1000 * Direction;     // 10m
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(EquipedWeapon);

    // LineTrace 검사
    bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    // 순간이동 예상 위치 얻기
    FVector BlinkLocation = (bIsHit && HitResult.bBlockingHit == true) ? HitResult.ImpactPoint : EndLocation;

    // 안전한 텔레포트 지점 계산
    GetWorld()->FindTeleportSpot(this, BlinkLocation, GetActorRotation());

    // 순간이동 실행
    TeleportTo(BlinkLocation, GetActorRotation());
}

void ALA_PlayerCharacter::EnhanceWeaponDamage(const float Duration)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("LA_PlayerCharacter -> EnhanceWeaponDamage 함수 호출됨"));

    if (EquipedWeapon == nullptr)
    {
        return;
    }

    //// 장비한 무기의 공격력 증가
    //EquipedWeapon
    FTimerDelegate Delegator = FTimerDelegate::CreateLambda([=]()
        {
            //// 장비한 무기의 공격력 감소
            //EquipedWeapon->
        });

    FTimerHandle SkillTimer;
    GetWorldTimerManager().SetTimer(SkillTimer, Delegator, Duration, false);
}

void ALA_PlayerCharacter::EnhanceMovementSpeed(const float Duration)
{
    // 이동 속도 증가
    WalkSpeed *= 2;
    SprintSpeed *= 2;
    CrouchSpeed *= 2;

    // 이동 속도 적용
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    GetCharacterMovement()->MaxWalkSpeed = (bIsSprint == true ? SprintSpeed : WalkSpeed);

    FTimerDelegate Delegator = FTimerDelegate::CreateLambda([&]()
        {
            // 이동 속도 감소
            WalkSpeed *= 0.5;
            SprintSpeed *= 0.5;
            CrouchSpeed *= 0.5;

            GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
            GetCharacterMovement()->MaxWalkSpeed = (bIsSprint == true ? SprintSpeed : WalkSpeed);
        });

    // 이동 속도 감소 예약
    FTimerHandle SkillTimer;
    GetWorldTimerManager().SetTimer(SkillTimer, Delegator, Duration, false);
}

#pragma endregion

void ALA_PlayerCharacter::SwapWeapon(int32 WeaponIndex)
{
    // 인덱스 유효성 확인
    if (WeaponClassNameIndexer.IsValidIndex(WeaponIndex) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Index (Out Of Range). No Weapon on %d Index Slot"), WeaponIndex);
        return;
    }

    // 선택한 무기 클래스의 이름 얻기
    FName WeaponClassName = WeaponClassNameIndexer[WeaponIndex];

    // 해당 클래스의 무기를 보유하고 있는지 확인
    if (OwnedWeapons.Contains(WeaponClassName) == true)
    {
        // 교체하려는 무기 얻기
        ALA_WeaponBase* NewWeapon = OwnedWeapons[WeaponClassName];

        // 선택된 무기 장착
        ILA_Holder::Execute_ActivateWeapon(this, NewWeapon);

    }
}

void ALA_PlayerCharacter::HealCharacter()
{
    // 체력 컴포넌트 확인
    if (HealthComponent != nullptr)
    {
        // 체력 회복
        HealthComponent->Heal(999999.f);
    }
}

void ALA_PlayerCharacter::OnPlayerDeath()
{
    // 사용자의 임의 카메라 회전 방지
    bUseControllerRotationYaw = false;

    // 캐릭터의 Collision 비활성화
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

    // 장착된 무기 해제
    ILA_Holder::Execute_DeactivateWeapon(this, EquipedWeapon);
    EquipedWeapon = nullptr;

    // 캐릭터의 기본 SkeletalMeshComponent 설정
    USkeletalMeshComponent* CharacterMesh = GetMesh();
    CharacterMesh->SetOwnerNoSee(false);
    CharacterMesh->SetCollisionProfileName(FName("Ragdoll"));
    CharacterMesh->SetSimulatePhysics(true);

    // Timeline에서 호출될 함수 설정
    FOnTimelineFloat TargetArmLengthCallback;
    TargetArmLengthCallback.BindUFunction(this, FName("UpdateCameraBoomTargetArmLength"));

    FOnTimelineVector CameraBoomRotationEulerCallback;
    CameraBoomRotationEulerCallback.BindUFunction(this, FName("UpdateCameraBoomRotation"));

    // 타임라인 커브 추가
    DeathCameraTimeline.AddInterpFloat(TargetArmLengthCurve, TargetArmLengthCallback);
    DeathCameraTimeline.AddInterpVector(CameraBoomRotationEulerCurve, CameraBoomRotationEulerCallback);

    // 타임라인 재생
    DeathCameraTimeline.SetTimelineLength(DeathCameraDuration);
    DeathCameraTimeline.PlayFromStart();
}

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
        // 뷰포트 담당 카메라 얻기
        UCameraComponent* Camera = GetCameraComponent();

		// 컨트롤러의 회전값 얻기
		//FRotator ControlRotation = GetControlRotation();
		FRotator CameraRotation = Camera->GetComponentRotation();

		// 컨트롤러 회전을 기준으로 방향 조정
		FVector Direction = InputVector.RotateAngleAxis(CameraRotation.Yaw, FVector::UpVector);

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

        if (EquipedWeapon != nullptr)
        {
            EquipedWeapon->Look(InputVector);
        }
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

void ALA_PlayerCharacter::AimingStartedAction()
{
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to start Aiming"));
        return;
    }

	// 토글 옵션이면서 조준 상태인 경우
	if (AimInputMode == EMovementInputMode::Toggle)
	{
		// 조준 상태 반전
        EquipedWeapon->bIsAiming = !EquipedWeapon->bIsAiming;
		return;
	}

	// 조준 상태로 설정
    EquipedWeapon->bIsAiming = true;
}

void ALA_PlayerCharacter::AimingCompletedAction()
{
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to stop Aiming"));
        return;
    }

	// Hold 옵션이면서 조준 상태의 경우
	if (AimInputMode == EMovementInputMode::Hold && EquipedWeapon->bIsAiming == true)
	{
		// 비조준 상태로 설정
        EquipedWeapon->bIsAiming = false;
		return;
	}
}

void ALA_PlayerCharacter::ReloadStartedAction()
{
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to Reload"));
        return;
    }

    // 재장전 실행
    EquipedWeapon->Reload();
}

#pragma endregion

float ALA_PlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    if (HealthComponent != nullptr)
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

    // 이동 속도 변경
    GetCharacterMovement()->MaxWalkSpeed = (bIsSprint ? SprintSpeed : WalkSpeed);
}

void ALA_PlayerCharacter::HideCharacterMesh()
{
    if (USkeletalMeshComponent* CharacterMesh = GetMesh())
    {
        //// Mesh 삭제
        //CharacterMesh->SetSkeletalMesh(nullptr);

        //// 렌더링되지 않도록 설정
        //CharacterMesh->SetVisibility(false, true);
        //CharacterMesh->SetHiddenInGame(true, true);
        //CharacterMesh->SetCastShadow(false);
        CharacterMesh->SetOwnerNoSee(true);

        // Collision 제거
        CharacterMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

        // Overlap Event 방지
        CharacterMesh->SetGenerateOverlapEvents(false);

        //// 컴포턴트 업데이트 방지
        //CharacterMesh->SetComponentTickEnabled(false);
    }
}

void ALA_PlayerCharacter::UpdateCameraBoomTargetArmLength(float Value)
{
    SpringArmComponent->TargetArmLength = Value;
}

void ALA_PlayerCharacter::UpdateCameraBoomRotation(FVector Value)
{
    SpringArmComponent->SetWorldRotation(FRotator::MakeFromEuler(Value));
}
