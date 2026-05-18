// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Projectile/LA_Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ALA_Projectile::ALA_Projectile()
{
    PrimaryActorTick.bCanEverTick = true;

    // 콜리전 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(10.0f);
    CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = CollisionComp;

    // 메시 설정
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);

    // 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.0f;
    ProjectileMovement->MaxSpeed = 3000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;

    Damage = 10.0f;
    InitialLifeSpan = 5.0f; // 5초 뒤 자동 소멸
}

void ALA_Projectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor != GetOwner())
    {
        if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
        {
            Destroy();
        }
    }
}

void ALA_Projectile::BeginPlay()
{
    Super::BeginPlay();
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ALA_Projectile::OnOverlapBegin);
}
