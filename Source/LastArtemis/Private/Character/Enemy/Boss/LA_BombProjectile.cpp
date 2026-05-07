// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/LA_BombProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ALA_BombProjectile::ALA_BombProjectile()
{
    ProjectileMovement->bShouldBounce = true;       // 바닥에 튕김
    ProjectileMovement->Bounciness = 0.4f;
    ProjectileMovement->ProjectileGravityScale = 1.2f; // 무겁게 떨어짐

    ExplosionRadius = 400.0f;
    FuseTime = 2.0f;
    Damage = 35.0f;
}

// Called when the game starts or when spawned
void ALA_BombProjectile::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &ALA_BombProjectile::Explode, FuseTime, false);
}

void ALA_BombProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 폭탄은 몸에 맞았을 때 바로 터지게 하고 싶다면 여기서 Explode()를 호출하면 됩니다.
    // 지금은 바닥에 굴러다니다 터지는 컨셉으로 비워두겠습니다.
}

void ALA_BombProjectile::Explode()
{
    // 1. 범위 데미지 적용
    // 맞은 대상이 Ally인지 Enemy인지 내부적으로 판단하여 데미지를 줍니다.
    UGameplayStatics::ApplyRadialDamage(
        this,
        Damage,
        GetActorLocation(),
        ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(), // 무시할 액터 (필요 시 자신이나 보스 추가)
        this,
        GetInstigatorController(),
        true // 가시성 체크 (벽 뒤 플레이어는 보호)
    );

    // 2. 이펙트 및 사운드
    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("Bomb Exploded at: %s"), *GetActorLocation().ToString());

    Destroy(); // 폭발 후 제거
}
