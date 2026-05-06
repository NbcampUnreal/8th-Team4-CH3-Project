#include "LA_WeaponBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

ALA_WeaponBase::ALA_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    Camera->FieldOfView = DefaultFOV;
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

    MaxSwayDegree = 4.f;
    SwayMultiplier = 1.5f;
    SwaySpeed = 10.f;
    SwayReturnSpeed = 15.f;

    bIsAiming = false;
    AimFOV = 60.f;
    AimInterpSpeed = 15.f;

    BaseDamage = 2.f;
	FireRate = 0.1f;
    MaxRange = 500.f;
	PelletCount = 1.f;
    SpreadAngle = 0.f;

    bCanFire = true;
}

void ALA_WeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (Mesh->DoesSocketExist(TEXT("FrontSight")) && Mesh->DoesSocketExist(TEXT("RearSight")))
    {
        FVector Front = Mesh->GetSocketTransform(TEXT("FrontSight"), RTS_Component).GetLocation();
        FVector Rear = Mesh->GetSocketTransform(TEXT("RearSight"), RTS_Component).GetLocation();

        FVector SightDirection = (Front - Rear).GetSafeNormal();
        FQuat DeltaRotation = FQuat::FindBetweenVectors(SightDirection, FVector::ForwardVector);
        AimMeshRotation = DeltaRotation.Rotator();

        FVector RotatedRear = AimMeshRotation.RotateVector(Rear);
        AimMeshLocation = FVector(AimDistanceOffset, 0.f, 0.f) - RotatedRear;
    }
}

void ALA_WeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CurrentSway = FMath::RInterpTo(CurrentSway, TargetSway, DeltaTime, SwaySpeed);
    SpringArm->SetRelativeRotation(CurrentSway);
    TargetSway = FMath::RInterpTo(TargetSway, FRotator::ZeroRotator, DeltaTime, SwayReturnSpeed);

    // Sway
    float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
    float CurrentFOV = Camera->FieldOfView;
    float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, AimInterpSpeed);
    Camera->SetFieldOfView(NewFOV);

    // Aim
    FVector TargetLocation = bIsAiming ? AimMeshLocation : DefaultMeshLocation;
    FVector CurrentLocation = Mesh->GetRelativeLocation();
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, AimInterpSpeed);
    Mesh->SetRelativeLocation(NewLocation);

    FRotator TargetRotation = bIsAiming ? AimMeshRotation : DefaultMeshRotation;
    FRotator CurrentRotation = Mesh->GetRelativeRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AimInterpSpeed);
    Mesh->SetRelativeRotation(NewRotation);
}

void ALA_WeaponBase::Look(FVector InputValue)
{
    CameraYaw += InputValue.X;
    CameraPitch = FMath::Clamp(CameraPitch + InputValue.Y, -89.f, 89.f);
    Camera->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));

    float PitchOffset = FMath::Clamp(InputValue.Y * SwayMultiplier, -MaxSwayDegree, MaxSwayDegree);
    float YawOffset = FMath::Clamp(-InputValue.X * SwayMultiplier, -MaxSwayDegree, MaxSwayDegree);
    TargetSway = FRotator(PitchOffset, YawOffset, 0.f);
}

void ALA_WeaponBase::StartFire()
{
	if (!bCanFire) return;

	bCanFire = false;
	FTimerDelegate ResetFire;
	ResetFire.BindLambda([this](){ bCanFire = true; });

    OnFire();
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, ResetFire, FireRate, false);
}

void ALA_WeaponBase::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	bCanFire = true;
}

void ALA_WeaponBase::Reload()
{
}

void ALA_WeaponBase::OnFire()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    FVector Start = GetActorLocation();
    FVector Direction = OwnerPawn->GetControlRotation().Vector();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerPawn);

    for (int32 i = 0; i < PelletCount; ++i)
    {
        FVector RandomizedDir = FMath::VRandCone(Direction, FMath::DegreesToRadians(SpreadAngle));
        FVector End = Start + (RandomizedDir * MaxRange);

        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

        if (bHit)
        {
            DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Green, false, 2.f, 0, 1.f);
            DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Green, false, 2.f);
        }
        else
        {
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f, 0, 1.f);
        }
    }
}
