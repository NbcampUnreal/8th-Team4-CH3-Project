#include "LA_WeaponBase.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

ALA_WeaponBase::ALA_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	BaseDamage = 20.0f;
	FireRate = 0.1f;
	MaxRange = 5000.0f;
	PelletCount = 1;
	SpreadAngle = 0.0f;

	bCanFire = true;
}

void ALA_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void ALA_WeaponBase::StartFire()
{
	if (bCanFire)
	{
		bCanFire = false;
		FTimerDelegate ResetFire;
		ResetFire.BindLambda([this]()
		{
			bCanFire = true;
		});

		Fire();
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, ResetFire, FireRate, false);
	}
}

void ALA_WeaponBase::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	bCanFire = true;
}

void ALA_WeaponBase::Fire()
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
			DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Green, false, 2.0f, 0, 1.0f);
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Green, false, 2.0f);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 1.0f);
		}
	}
}

void ALA_WeaponBase::Reload()
{
}
