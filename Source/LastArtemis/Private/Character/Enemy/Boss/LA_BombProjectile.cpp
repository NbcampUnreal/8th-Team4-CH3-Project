// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/LA_BombProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ALA_BombProjectile::ALA_BombProjectile()
{
    if (ProjectileMovement)
    {
        ProjectileMovement->bShouldBounce = true;       // 바닥에 튕김 활성화
        ProjectileMovement->Bounciness = 0.4f;          // 탄성값
        ProjectileMovement->ProjectileGravityScale = 1.2f; // 무겁게 낙하

        // 아래 옵션을 주면 튕길 때마다 마찰력 때문에 자연스럽게 멈춰 섭니다.
        ProjectileMovement->Friction = 0.5f;
    }

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

    if (OtherActor && OtherActor != GetOwner() && OtherActor->ActorHasTag(FName("Team.Ally")))
    {
        // 타이머를 취소하고 즉시 폭발
        GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandle);
        Explode();
    }
}

void ALA_BombProjectile::Explode()
{
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);
    if (GetOwner()) IgnoreActors.Add(GetOwner());
    if (GetInstigator()) IgnoreActors.Add(GetInstigator());

    //  범위 데미지 적용
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

    //  이펙트 및 사운드
    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("Bomb Exploded at: %s"), *GetActorLocation().ToString());

    Destroy(); // 폭발 후 제거
}
