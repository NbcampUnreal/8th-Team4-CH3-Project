// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/LA_PlayerCharacter.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"	        // UCharacterMovementComponent
#include "GameFramework/SpringArmComponent.h"                   // USpringArmComponent
#include "Components/CapsuleComponent.h"                        // UCapsuleComponent
#include "Camera/CameraComponent.h"		                        // UCameraComponent
#include "Item/LA_InventoryComponent.h"                         // ULA_InventoryComponent
#include "EnhancedInputComponent.h"		                        // UEnhancedInputComponent, FInputActionValue
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/LA_DefaultPlayerController.h"		        // ALA_DefaultPlayerController
#include "Character/Player/Component/LA_HealthComponent.h"      // ULA_HealthComponent
#include "LastArtemis/Weapon/LA_WeaponBase.h"                   // ALA_WeaponBase
#include "GameMode/LA_GameInstance.h"                           // ULA_GameInstance
#include "GameMode/LA_GameModeBase.h"                           // ULA_GameModeBase
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Object/LA_Interactable.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"

// Sets default values
ALA_PlayerCharacter::ALA_PlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 체력 관련 컴포넌트 부착
    HealthComponent = CreateDefaultSubobject<ULA_HealthComponent>(FName("HealthComponent"));

    // 인벤토리 컴포넌트 부착
    InventoryComponent = CreateDefaultSubobject<ULA_InventoryComponent>(TEXT("InventoryComponent"));

    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    MovementComponent->MaxWalkSpeed = WalkSpeed; // 캐릭터 기본 이동 속도 변경 (10.8 km/s)
    MovementComponent->bUseSeparateBrakingFriction = true;  // 입력 종료 시 GroundFriction 대신 BrakingFriction을 사용하여 감속 적용
    MovementComponent->BrakingFriction = 0.5f;      // 입력 종료 시 적용되는 마찰력 설정
    MovementComponent->BrakingFrictionFactor = 2;   // Friction 적용 배율 설정
    MovementComponent->BrakingDecelerationWalking = 256;    // 입력 없을 경우에 적용되는 고정 감속량 설정

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

    // GameInstance를 가져와서 설정을 적용.
    ULA_GameInstance* GI = Cast<ULA_GameInstance>(GetGameInstance());
    if (GI)
    {
        GI->ApplySettingsToCharacter();
    }

    // 캐릭터의 기본 USkeletalMeshComponent 숨기기
    HideCharacterMesh();

    // 무기 인스턴스 생성
    SpawnWeaponActor();

    // 초기 무기의 유효성 확인 획득
    if (initialWeaponData.IsValid() == true)
    {
        // 동기 방식을 사용하여 초기 무기 획득
        UAssetManager& AssetManager = UAssetManager::Get();
        AssetManager.GetStreamableManager().LoadSynchronous(AssetManager.GetPrimaryAssetPath(initialWeaponData));
        OnCompletedAsyncLoadWeaponDataAsset(initialWeaponData);

        // 보유한 무기의 개수 확인
        if (WeaponIDIndexer.IsEmpty() == false)
        {
            // 0번 인덱스의 무기 장착
            SwapWeapon(0);
        }
    }

    HealthComponent->OnContaminationChanged.AddUObject(this, &ALA_PlayerCharacter::OnContaminationChanged);
    HealthComponent->OnDeath.AddDynamic(this, &ALA_PlayerCharacter::OnPlayerDeath);
    OnMovementSpeedChanged.AddDynamic(this, &ALA_PlayerCharacter::UpdateMovementSpeed);
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
            // Interact
            if (LA_Controller->InteractInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->InteractInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::InteractStartedAction);
            }
		    // CommandTarget
		    if (LA_Controller->CommandTargetAction != nullptr)
		    {
		        enhancedInputComponent->BindAction(LA_Controller->CommandTargetAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::CommandTargetStartedAction);
		        enhancedInputComponent->BindAction(LA_Controller->CommandTargetAction, ETriggerEvent::Triggered, this, &ALA_PlayerCharacter::CommandTargetTriggeredAction);
		        enhancedInputComponent->BindAction(LA_Controller->CommandTargetAction, ETriggerEvent::Completed, this, &ALA_PlayerCharacter::CommandTargetCompletedAction);

		    }
            // Pause
            if (LA_Controller->PauseInputAction != nullptr)
            {
                enhancedInputComponent->BindAction(LA_Controller->PauseInputAction, ETriggerEvent::Started, this, &ALA_PlayerCharacter::PauseAction);
            }
            // Item QuickSlot InputAction
            for (int32 i = 0; i < LA_Controller->ItemQuickSlotActions.Num(); ++i)
            {
                if (LA_Controller->ItemQuickSlotActions[i] != nullptr)
                {
                    enhancedInputComponent->BindAction(LA_Controller->ItemQuickSlotActions[i], ETriggerEvent::Started, this, &ALA_PlayerCharacter::UseQuickSlot, i);
                }
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

void ALA_PlayerCharacter::OnDeathCameraTimelineFinished()
{
    ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!LA_GameMode)
        return;

    LA_GameMode->OnGameOver();
}

UCameraComponent* ALA_PlayerCharacter::GetCameraComponent() const
{
    return EquipedWeapon == nullptr ? DeathCamera.Get() : EquipedWeapon->GetFirstPersonCamera();
}

FHitResult ALA_PlayerCharacter::LineTraceForward(float distance)
{
    UCameraComponent* Camera = GetCameraComponent();

    // LineTrace 실행
    FHitResult HitResult;
    FVector StartLocation = Camera->GetComponentLocation();
    FVector Direction = Camera->GetForwardVector();
    FVector EndLocation = StartLocation + distance * Direction;	// 100 m 검사
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(EquipedWeapon);

    GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    return HitResult;
}

void ALA_PlayerCharacter::SwapWeapon(int32 WeaponIndex)
{
    if (EquipedWeapon->CurrentState != EWeaponState::Idle) return;

    // 인덱스 유효성 확인
    if (WeaponIDIndexer.IsValidIndex(WeaponIndex) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Index (Out Of Range). No Weapon on %d Index Slot"), WeaponIndex);
        return;
    }

    // 선택한 무기 클래스의 이름 얻기
    FPrimaryAssetId WeaponID = WeaponIDIndexer[WeaponIndex];

    // 해당 클래스의 무기를 보유하고 있는지 확인
    if (OwnedWeapons.Contains(WeaponID) == true)
    {
        // 교체하려는 무기 얻기
        ULA_WeaponData* NewWeaponData = OwnedWeapons[WeaponID];

        // 선택된 무기 장착
        ILA_Holder::Execute_ActivateWeapon(this, NewWeaponData);
    }
}

void ALA_PlayerCharacter::OnPlayerDeath()
{
    // 사용자의 임의 카메라 회전 방지
    bUseControllerRotationYaw = false;

    // 캐릭터의 Collision 비활성화
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

    // 장착된 무기 해제
    ILA_Holder::Execute_DeactivateWeapon(this);
    EquipedWeapon = nullptr;

    // 캐릭터의 기본 SkeletalMeshComponent 설정
    USkeletalMeshComponent* CharacterMesh = GetMesh();
    CharacterMesh->SetOwnerNoSee(false);        // 카메라에 보이도록 설정
    CharacterMesh->SetCollisionProfileName(FName("Ragdoll"));   // Collision Preset 설정
    CharacterMesh->SetSimulatePhysics(true);        // 물리법칙에 따라 쓰러지도록 설정

    // Timeline에서 호출될 함수 설정
    FOnTimelineFloat TargetArmLengthCallback;
    TargetArmLengthCallback.BindUFunction(this, FName("UpdateCameraBoomTargetArmLength"));      // TargetArmLength 길이 조정

    FOnTimelineVector CameraBoomRotationEulerCallback;
    CameraBoomRotationEulerCallback.BindUFunction(this, FName("UpdateCameraBoomRotation"));     // Rotation 조정

    FOnTimelineEvent DeathCameraFinishedCallback;
    DeathCameraFinishedCallback.BindUFunction(this, FName("OnDeathCameraTimelineFinished"));    // 타임라인 종료 시 호출될 함수 바인딩

    // 타임라인 커브 추가
    DeathCameraTimeline.AddInterpFloat(TargetArmLengthCurve, TargetArmLengthCallback);
    DeathCameraTimeline.AddInterpVector(CameraBoomRotationEulerCurve, CameraBoomRotationEulerCallback);
    DeathCameraTimeline.SetTimelineFinishedFunc(DeathCameraFinishedCallback);

    // 타임라인 재생
    DeathCameraTimeline.SetTimelineLength(DeathCameraDuration);
    DeathCameraTimeline.PlayFromStart();
}

#pragma region Derived From ILA_Holder

void ALA_PlayerCharacter::AddWeaponToPawn_Implementation(FPrimaryAssetId WeaponDataID)
{
    // DataAsset의 ID가 유효한지 확인
    if (WeaponDataID.IsValid() == false)
    {
        return;
    }
    UAssetManager& AssetManager = UAssetManager::Get();

    // 기존에 로드된 DataAsset이 존재하는지 확인
    ULA_WeaponData* WeaponDataAsset = UAssetManager::Get().GetPrimaryAssetObject<ULA_WeaponData>(WeaponDataID);
    if (WeaponDataAsset == nullptr)
    {
        // DataAsset의 ID를 통하여 데이터 비동기 로드
        TArray<FName> Bundles;
        FStreamableDelegate OnCompletedAsyncLoad = FStreamableDelegate::CreateUObject(this, &ALA_PlayerCharacter::OnCompletedAsyncLoadWeaponDataAsset, WeaponDataID);

        DataAssetLoadingHandler = AssetManager.LoadPrimaryAsset(WeaponDataID, Bundles, OnCompletedAsyncLoad);
        return;
    }

    // 동기 방식 로드
    AssetManager.GetStreamableManager().LoadSynchronous(AssetManager.GetPrimaryAssetPath(WeaponDataID));
    OnCompletedAsyncLoadWeaponDataAsset(WeaponDataID);
}

void ALA_PlayerCharacter::ActivateWeapon_Implementation(ULA_WeaponData* WeaponData)
{
    if (IsValid(WeaponData) == false)
    {
        return;
    }

    FString DebugMessage = FString::Printf(TEXT("무기(%s) 장착"), *WeaponData->GetPrimaryAssetId().ToString());
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, DebugMessage);

    //// 현재 장착 중인 무기를 활성화하는 경우
    //if (Weapon == EquipedWeapon)
    //{
    //    // 함수 조기 종료
    //    return;
    //}
    //// 다른 무기 활성화 시 현재 무기 비활성화
    //if (EquipedWeapon != nullptr)
    //{
    //    ILA_Holder::Execute_DeactivateWeapon(this, EquipedWeapon);
    //}

    // 장착 무기 교체
    EquipedWeapon->SetWeaponData(WeaponData);
    EquipedWeapon->Draw();

    // HUD 업데이트
    ILA_Holder::Execute_UpdateHUDWidgetOnActor(this, EquipedWeapon);

    return;
}

void ALA_PlayerCharacter::DeactivateWeapon_Implementation()
{
    if (EquipedWeapon == nullptr)
    {
        return;
    }

    // 무기 조준 상태 해제 및 발사 중지
    EquipedWeapon->StopAiming();
    EquipedWeapon->StopFire();

    // 할당된 무기 데이터 제거
    EquipedWeapon->SetWeaponData(nullptr);
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
    FHitResult HitResult = LineTraceForward(10000);     // 100 m 검사

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
    FHitResult HitResult = LineTraceForward(1000);      // 10 m 검사

    // 순간이동 예상 위치 얻기
    FVector BlinkLocation = HitResult.bBlockingHit == true ? HitResult.ImpactPoint : HitResult.TraceEnd;

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

    if (OnMovementSpeedChanged.IsBound() == true)
    {
        OnMovementSpeedChanged.Broadcast();
    }

    FTimerDelegate Delegator = FTimerDelegate::CreateLambda([&]()
        {
            // 이동 속도 감소
            WalkSpeed /= 2;
            SprintSpeed /= 2;
            CrouchSpeed /= 2;

            if (OnMovementSpeedChanged.IsBound() == true)
            {
                OnMovementSpeedChanged.Broadcast();
            }
        });

    // 이동 속도 감소 예약
    FTimerHandle SkillTimer;
    GetWorldTimerManager().SetTimer(SkillTimer, Delegator, Duration, false);
}

#pragma endregion

void ALA_PlayerCharacter::UpdateMovementSpeed()
{
    GetCharacterMovement()->MaxWalkSpeed = (bIsSprint == true ? SprintSpeed : WalkSpeed) * MovementSpeedFactor;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed * MovementSpeedFactor;
}

void ALA_PlayerCharacter::UpdateCameraBoomTargetArmLength(float Value)
{
    SpringArmComponent->TargetArmLength = Value;
}

void ALA_PlayerCharacter::UpdateCameraBoomRotation(FVector Value)
{
    SpringArmComponent->SetWorldRotation(FRotator::MakeFromEuler(Value));
}

void ALA_PlayerCharacter::OnCompletedAsyncLoadWeaponDataAsset(FPrimaryAssetId WeaponDataID)
{
    ULA_WeaponData* WeaponDataAsset = UAssetManager::Get().GetPrimaryAssetObject<ULA_WeaponData>(WeaponDataID);

    if (WeaponDataAsset == nullptr)
    {
        return;
    }

    // 중복 무기 획득 검사
    if (OwnedWeapons.Contains(WeaponDataID) == true)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("이미 소유하고 있는 무기입니다. - %s"), *WeaponDataID.ToString()));
        ULA_WeaponData* DuplicatedData = OwnedWeapons[WeaponDataID];

        // 무기의 총알 채워넣기
        if (EquipedWeapon != nullptr)
        {
            //EquipedWeapon->RefillAmmo();
        }
        return;
    }
    else
    {
        // 보유한 무기 목록에 추가
        OwnedWeapons.Add(WeaponDataID, WeaponDataAsset);
        WeaponIDIndexer.Add(WeaponDataID);
    }
}

void ALA_PlayerCharacter::OnContaminationChanged(float CurrentContamination, float MaxContamination)
{
    float ContaminationPercentage = CurrentContamination / MaxContamination;
    MovementSpeedFactor = FMath::Lerp(0.7f, 1.f, ContaminationPercentage);
    OnMovementSpeedChanged.Broadcast();
}

#pragma region Select Enemy

// 화면에 보이는 적군 목록 반환
TArray<AActor*> ALA_PlayerCharacter::GetVisibleEnemies()
{
    // Team.Enemy 태그 가진 액터만 가져오기
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);



    // AllActor Actor 개수 확인
    UE_LOG(LogTemp, Warning, TEXT("AllActors Count: %d"), AllActors.Num());


    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    // 보이는 적 저장
    TArray<AActor*> VisibleEnemies;

    APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PC) return VisibleEnemies;

    for (AActor* Actor : AllActors)
    {
        // GameplayTag 인터페이스 확인
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
        if (!TagInterface) continue;

        FGameplayTagContainer OwnedTags;
        TagInterface->GetOwnedGameplayTags(OwnedTags);
        // 태그 확인용 로그
        UE_LOG(LogTemp, Warning, TEXT("Actor: %s, Tags: %s"), *Actor->GetName(), *OwnedTags.ToString());
        // Team.Enemy 태그 가진 액터만 필터링
        if (!TagInterface->HasMatchingGameplayTag(EnemyTag)) continue;

        FVector2D ScreenPos;
        // 화면 안에 위치가 투영되는 적군만 필터링
        if (PC->ProjectWorldLocationToScreen(Actor->GetActorLocation(), ScreenPos))
        {
            VisibleEnemies.Add(Actor);
        }
    }
    return VisibleEnemies;



}

// 크로스헤어(화면 중앙)에 가장 가까운 적군 반환
AActor* ALA_PlayerCharacter::GetCrosshairTarget(const TArray<AActor*>& Enemies)
{
    APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PC) return nullptr;

    // 화면 해상도 가져오기
    int32 SizeX, SizeY;
    PC->GetViewportSize(SizeX, SizeY);

    // 화면 중앙 좌표 계산
    FVector2D ScreenCenter(SizeX / 2.f, SizeY / 2.f);

    AActor* ClosestEnemy = nullptr;
    float MinDistance = FLT_MAX;

    for (AActor* Enemy : Enemies)
    {
        FVector2D ScreenPos;
        if (PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPos))
        {
            // 화면 중앙과 적군 스크린 좌표 사이의 거리 계산
            float Distance = FVector2D::Distance(ScreenPos, ScreenCenter);

            // 더 가까운 적군이면 업데이트
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestEnemy = Enemy;
            }
        }
    }
    return ClosestEnemy;


}

void ALA_PlayerCharacter::SetTarget(AActor* Target)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALA_AllyAI::StaticClass(), AllActors);

    for (AActor* Ally : AllActors)
    {
        ALA_AllyAI* AllyAI = Cast<ALA_AllyAI>(Ally);
        if (!AllyAI) continue;

        if (ALA_AllyAIController* AIController = Cast<ALA_AllyAIController>(AllyAI->GetController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsObject(FName("TargetActor"), Target);
                Blackboard->SetValueAsBool(FName("IsCommandedTarget"), true);
                UE_LOG(LogTemp, Warning, TEXT("Command Complete: Target - %s"), *Target->GetName());
            }
        }


    }
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

    // 달리기 상태에서 총이 발사되는 것을 방지
    if (bIsSprint)
    {
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

    // 달리기 상태에서 줌 방지
    if (bIsSprint == true)
    {
        return;
    }

	// 토글 옵션이면서 조준 상태인 경우
	if (AimInputMode == EMovementInputMode::Toggle)
	{
		// 조준 상태 반전
        if (EquipedWeapon->IsAiming())
            EquipedWeapon->StopAiming();
        else
            EquipedWeapon->StartAiming();
		return;
	}

	// 조준 상태로 설정
    EquipedWeapon->StartAiming();
}

void ALA_PlayerCharacter::AimingCompletedAction()
{
    if (Controller == nullptr || EquipedWeapon == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Controller or Weapon to stop Aiming"));
        return;
    }

	// Hold 옵션이면서 조준 상태의 경우
	if (AimInputMode == EMovementInputMode::Hold && EquipedWeapon->IsAiming())
	{
		// 비조준 상태로 설정
        EquipedWeapon->StopAiming();
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

void ALA_PlayerCharacter::InteractStartedAction()
{
    if (Controller == nullptr)
        return;

    // 바라보는 방향 객체 검사
    FHitResult HitResult = LineTraceForward(300);      // 1 m 검사

    AActor* HitActor = HitResult.GetActor();
    if (!IsValid(HitActor))
        return;

    // 상호작용 인터페이스가 없는 Actor면 무시
    if (!HitActor->GetClass()->ImplementsInterface(ULA_Interactable::StaticClass()))
        return;

    ILA_Interactable::Execute_Interact(HitActor, this);
}


void ALA_PlayerCharacter::CommandTargetStartedAction()
{
    // V 누르는 순간 실행
    // TODO: UI 표시 로직 추가 예정


    //UE_LOG(LogTemp, Warning, TEXT("CommandTarget Started!"));


}

void ALA_PlayerCharacter::CommandTargetTriggeredAction()
{
    TArray<AActor*> Enemies = GetVisibleEnemies();
    AActor* NewAimedEnemy = GetCrosshairTarget(Enemies);



    if (CurrentAimedEnemy != NewAimedEnemy)
    {
        if (CurrentAimedEnemy.IsValid())
        {
            if (USkeletalMeshComponent* OldMesh = CurrentAimedEnemy->FindComponentByClass<USkeletalMeshComponent>())
            {
                OldMesh->SetOverlayMaterial(nullptr);
            }
        }
    }

    if (NewAimedEnemy && OutlineMaterial)
    {
        if (USkeletalMeshComponent* NewMesh = NewAimedEnemy->FindComponentByClass<USkeletalMeshComponent>())
        {
            NewMesh->SetOverlayMaterial(OutlineMaterial);
        }
    }

    CurrentAimedEnemy = NewAimedEnemy;

}



void ALA_PlayerCharacter::CommandTargetCompletedAction()
{

    AActor* FinalTarget = CurrentAimedEnemy.Get();


    if (FinalTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target: %s"), *FinalTarget->GetName());
        SetTarget(FinalTarget);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("공격 명령 취소"));
    }

    if (CurrentAimedEnemy.IsValid())
    {
        if (USkeletalMeshComponent* AimedMesh = CurrentAimedEnemy->FindComponentByClass<USkeletalMeshComponent>())
        {
            AimedMesh->SetOverlayMaterial(nullptr);
        }
    }
    CurrentAimedEnemy = nullptr;

    // TODO: 조준 UI 끄기
}

void ALA_PlayerCharacter::PauseAction()
{
    if (Controller == nullptr) return;

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC)
        return;

    ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM)
        return;

    // 일시 정지 상태면 게임 재개
    if (UGameplayStatics::IsGamePaused(GetWorld()))
    {
        if (CurrentPauseMenu)
        {
            CurrentPauseMenu->RemoveFromParent();
            CurrentPauseMenu = nullptr;
        }

        GM->ResumeGame();

        return;
    }

    // 일시 정지 로직 실행
    GM->PauseGame();

    // 일시정지 UI 생성 및 출력
    if (PauseMenuWidgetClass)
    {
        CurrentPauseMenu = CreateWidget<UUserWidget>(PC, PauseMenuWidgetClass);
        if (CurrentPauseMenu)
        {
            CurrentPauseMenu->AddToViewport();

            // 입력 모드 전환
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(CurrentPauseMenu->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

void ALA_PlayerCharacter::InventoryAction()
{
    if (Controller == nullptr) return;
    // 게임 일시정지 로직 실행
    if (ALA_GameModeBase* GM = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->PauseGame();
    }

    // 인벤토리 UI 생성 및 출력
    if (InventoryWidgetClass)
    {
        UUserWidget* Inventory = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (Inventory)
        {
            Inventory->AddToViewport();

            // 입력 모드 전환
            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(Inventory->TakeWidget());
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
    }
}

void ALA_PlayerCharacter::UseQuickSlot(int32 SlotIndex)
{
    if (!InventoryComponent)
        return;

    if (SlotIndex < 0)
        return;

    InventoryComponent->UseQuickItem(SlotIndex, this);
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

    // 달리기 전환 시 조준 상태 해제
    if (bIsSprint == true && EquipedWeapon != nullptr)
    {
        EquipedWeapon->StopAiming();
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

void ALA_PlayerCharacter::SpawnWeaponActor()
{
    if (EquipedWeapon != nullptr)
    {
        return;
    }

    // 무기 인스턴스 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;   // 소유자 설정
    SpawnParams.Instigator = this;
    EquipedWeapon = GetWorld()->SpawnActor<ALA_WeaponBase>(GetActorLocation(), GetActorRotation(), SpawnParams);

    // 무기 액터를 캐릭터의 자식으로 붙이기
    EquipedWeapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    // 무기 액터 위치 조정
    EquipedWeapon->SetActorRelativeLocation(FVector(0, 0, BaseEyeHeight));
}

void ALA_PlayerCharacter::RefillWeaponAmmo()
{
    if (!EquipedWeapon)
        return;

    EquipedWeapon->RefillAmmo();
}
