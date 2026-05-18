#include "LA_WeaponBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Curves/CurveVector.h"
#include "LastArtemis/Character/LA_Holder.h"
#include "Character/Player/LA_PlayerCharacter.h"

ALA_WeaponBase::ALA_WeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    Camera->bUsePawnControlRotation = false;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Camera);
    SpringArm->TargetArmLength = 0.f;
    SpringArm->bUsePawnControlRotation = false;

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(SpringArm);

    CameraPitch = 0.f;
    CameraYaw = 0.f;
    TargetSway = FRotator::ZeroRotator;
    CurrentSway = FRotator::ZeroRotator;
    CurrentSpareAmmo = 90;
    CurrentState = EWeaponState::Idle;
}

void ALA_WeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sway
    CurrentSway = FMath::RInterpTo(CurrentSway, TargetSway, DeltaTime, WeaponData->SwaySpeed);
    SpringArm->SetRelativeRotation(CurrentSway);
    TargetSway = FMath::RInterpTo(TargetSway, FRotator::ZeroRotator, DeltaTime, WeaponData->SwayReturnSpeed);

    // Aim - FOV
    float TargetFOV = bIsAiming ? WeaponData->AimFOV : WeaponData->DefaultFOV;
    float CurrentFOV = Camera->FieldOfView;
    float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, WeaponData->AimInterpSpeed);
    Camera->SetFieldOfView(NewFOV);

    // Aim - Location
    FVector TargetLocation = bIsAiming ? AimMeshLocation : WeaponData->DefaultMeshLocation;
    FVector CurrentLocation = Mesh->GetRelativeLocation();
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, WeaponData->AimInterpSpeed);
    Mesh->SetRelativeLocation(NewLocation);

    // Aim - Rotation
    FRotator TargetRotation = bIsAiming ? AimMeshRotation : WeaponData->DefaultMeshRotation;
    FRotator CurrentRotation = Mesh->GetRelativeRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WeaponData->AimInterpSpeed);
    Mesh->SetRelativeRotation(NewRotation);

    // Spread Angle Recovery
    if (CurrentSpreadAngle > 0.f)
    {
        CurrentSpreadAngle = FMath::FInterpTo(CurrentSpreadAngle, 0.f, DeltaTime, 1.f / WeaponData->FireRate);
    }

    // Recoil
    if (TargetRecoil != CurrentRecoil)
    {
        // 현재 프레임에서 이동해야 할 반동량 계산
        FRotator NextRecoil = FMath::RInterpTo(CurrentRecoil, TargetRecoil, DeltaTime, WeaponData->RecoilSpeed);
        FRotator RecoilDeltaThisFrame = NextRecoil - CurrentRecoil;
        CurrentRecoil = NextRecoil;

        // 계산된 프레임당 반동량을 실제 카메라 회전값에 추가
        CameraPitch = FMath::Clamp(CameraPitch + RecoilDeltaThisFrame.Pitch, -89.f, 89.f);
        CameraYaw += RecoilDeltaThisFrame.Yaw;
        Camera->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));
    }
}

void ALA_WeaponBase::SetWeaponData(ULA_WeaponData* NewWeaponData)
{
    if (NewWeaponData == nullptr) return;

    WeaponData = NewWeaponData;
    Camera->FieldOfView = WeaponData->DefaultFOV;
    Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
    Mesh->SetAnimInstanceClass(WeaponData->WeaponAnimBP);

    bIsAiming = false;
    CurrentState = EWeaponState::Idle;
    CurrentMagazineAmmo = WeaponData->MaxMagazineSize;
    CurrentSpreadAngle = WeaponData->DefaultSpreadAngle;
    ResetRecoil();

    // 소켓을 기준으로 조준선 오프셋 계산
    if (Mesh->DoesSocketExist(TEXT("FrontSight")) && Mesh->DoesSocketExist(TEXT("RearSight")))
    {
        FVector Front = Mesh->GetSocketTransform(TEXT("FrontSight"), RTS_Component).GetLocation();
        FVector Rear = Mesh->GetSocketTransform(TEXT("RearSight"), RTS_Component).GetLocation();

        FVector SightDirection = (Front - Rear).GetSafeNormal();
        FQuat DeltaRotation = FQuat::FindBetweenVectors(SightDirection, FVector::ForwardVector);
        AimMeshRotation = DeltaRotation.Rotator();

        FVector RotatedRear = AimMeshRotation.RotateVector(Rear);
        AimMeshLocation = FVector(WeaponData->AimDistanceOffset, 0.f, 0.f) - RotatedRear;
    }
}

void ALA_WeaponBase::Draw()
{
    CurrentState = EWeaponState::Draw;

    if (UAnimMontage* AnimMontage = WeaponData->DrawMontage)
    {
        float Duration = Mesh->GetAnimInstance()->Montage_Play(AnimMontage);
        GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &ALA_WeaponBase::ResetState, Duration, false);
    }
}

void ALA_WeaponBase::Look(FVector InputValue)
{
    CameraYaw += InputValue.X;
    CameraPitch = FMath::Clamp(CameraPitch - InputValue.Y, -89.f, 89.f);
    Camera->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));

    // 마우스 이동 반대 방향으로 무기가 움직이는 Sway 방향 설정
    float PitchOffset = FMath::Clamp(InputValue.Y * WeaponData->SwayMultiplier, -WeaponData->MaxSwayDegree, WeaponData->MaxSwayDegree);
    float YawOffset = FMath::Clamp(-InputValue.X * WeaponData->SwayMultiplier, -WeaponData->MaxSwayDegree, WeaponData->MaxSwayDegree);
    TargetSway = FRotator(PitchOffset, YawOffset, 0.f);
}

void ALA_WeaponBase::StartFire()
{
    if (!CanFire()) return;

    Fire();
    GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ALA_WeaponBase::Fire, WeaponData->FireRate, true);
}

void ALA_WeaponBase::StopFire()
{
    GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}

void ALA_WeaponBase::Reload()
{
    if (CurrentState != EWeaponState::Idle || CurrentMagazineAmmo == WeaponData->MaxMagazineSize || CurrentSpareAmmo <= 0) return;

    CurrentState = EWeaponState::Reload;
    bIsAiming = false; // 장전 시 조준 강제 해제

    if (UAnimMontage* AnimMontage = WeaponData->ReloadMontage)
    {
        Mesh->GetAnimInstance()->Montage_Play(AnimMontage);
    }
}

float ALA_WeaponBase::GetDynamicSpreadAngle() const
{
    float DynamicSpreadAngle = WeaponData->DefaultSpreadAngle;

    if (bIsAiming)
    {
        DynamicSpreadAngle *= 0.5f; // 조준 시 절반 감소
    }

    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        float CurrentSpeed = OwnerCharacter->GetVelocity().Size2D();
        float MaxSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
        float SpeedRatio = CurrentSpeed / MaxSpeed;
        DynamicSpreadAngle += (SpeedRatio * 2.f); // 이동 시 최대 2배 증가
    }

    DynamicSpreadAngle += CurrentSpreadAngle; // 누적 합
    return FMath::Clamp(DynamicSpreadAngle, WeaponData->MinSpreadAngle, WeaponData->MaxSpreadAngle);
}

bool ALA_WeaponBase::CanFire() const
{
    return CurrentState == EWeaponState::Idle && CurrentMagazineAmmo > 0;
}

void ALA_WeaponBase::Fire()
{
    if (CurrentMagazineAmmo <= 0)
    {
        StopFire();
        return;
    }

    CurrentState = EWeaponState::Fire;

    if (UAnimMontage* AnimMontage = WeaponData->FireMontage)
    {
        Mesh->GetAnimInstance()->Montage_Play(AnimMontage);
    }

    // 총알 소모
    CurrentMagazineAmmo--;

    // UI 갱신
    if (GetOwner() && GetOwner()->Implements<ULA_Holder>())
    {
        ILA_Holder::Execute_UpdateHUDWidgetOnActor(GetOwner(), this);
    }

    HitScan();
    GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &ALA_WeaponBase::ResetState, WeaponData->FireRate, false);
}

void ALA_WeaponBase::HitScan()
{
    // 카메라 기준 타겟 위치 계산
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

    FVector StartLocation = Mesh->GetSocketLocation(TEXT("Muzzle"));
    FVector TargetLocation = ILA_Holder::Execute_GetFocusLocation(OwnerPawn);
    FVector FireDirection = (TargetLocation - StartLocation).GetSafeNormal();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerPawn) Params.AddIgnoredActor(OwnerPawn);

    float SpreadAngle = GetDynamicSpreadAngle();

    for (int32 i = 0; i < WeaponData->PelletCount; ++i)
    {
        FVector RandomizedDirection = FMath::VRandCone(FireDirection, FMath::DegreesToRadians(SpreadAngle));
        FVector EndLocation = StartLocation + (RandomizedDirection * WeaponData->MaxRange);

        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

        if (bHit)
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                UGameplayStatics::ApplyPointDamage(HitActor, WeaponData->BaseDamage, RandomizedDirection, HitResult, OwnerController, this, UDamageType::StaticClass());
            }

            DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Red, false, 2.f, 0, 1.f);
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red, false, 3.f);

            UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
                WeaponData->DecalMaterial,
                FVector(10.f, 10.f, 10.f),
                HitResult.GetComponent(),
                HitResult.BoneName,
                HitResult.ImpactPoint,
                HitResult.ImpactNormal.Rotation(),
                EAttachLocation::KeepWorldPosition,
                10.f
            );

            if (Decal)
            {
                if (UMaterialInstanceDynamic* DynamicMaterial = Decal->CreateDynamicMaterialInstance())
                {
                    float RandomFrame = FMath::RandRange(0, 63);
                    DynamicMaterial->SetScalarParameterValue(FName("AtlasIndex"), RandomFrame);
                }
            }
        }
        else
        {
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 2.f, 0, 1.f);
        }
    }

    // 발사 후 반동 적용
    ApplyRecoil();

    // 발사 후 반동에 의한 탄착 범위 증가
    CurrentSpreadAngle = FMath::Clamp(CurrentSpreadAngle + WeaponData->SpreadIncrement, 0.f, WeaponData->MaxSpreadAngle);
}

void ALA_WeaponBase::UpdateAmmo()
{
    int32 AmmoNeeded = WeaponData->MaxMagazineSize - CurrentMagazineAmmo;
    int32 AmmoToReload = FMath::Min(AmmoNeeded, CurrentSpareAmmo);

    CurrentMagazineAmmo += AmmoToReload;
    CurrentSpareAmmo -= AmmoToReload;
    CurrentState = EWeaponState::Idle;

    // UI 갱신
    if (GetOwner() && GetOwner()->Implements<ULA_Holder>())
    {
        ILA_Holder::Execute_UpdateHUDWidgetOnActor(GetOwner(), this);
    }
}

void ALA_WeaponBase::ApplyRecoil()
{
    if (WeaponData->RecoilCurve)
    {
        FVector RecoilDelta = WeaponData->RecoilCurve->GetVectorValue(static_cast<float>(CurrentShotCount));
        TargetRecoil.Pitch += RecoilDelta.Y;
        TargetRecoil.Yaw += RecoilDelta.X;
    }

    CurrentShotCount++;
    GetWorld()->GetTimerManager().SetTimer(RecoilResetTimerHandle, this, &ALA_WeaponBase::ResetRecoil, WeaponData->RecoilResetTime, false);
}

void ALA_WeaponBase::ResetRecoil()
{
    CurrentShotCount = 0;
    TargetRecoil = FRotator::ZeroRotator;
    CurrentRecoil = FRotator::ZeroRotator;
}

void ALA_WeaponBase::ResetState()
{
    CurrentState = EWeaponState::Idle;
}
