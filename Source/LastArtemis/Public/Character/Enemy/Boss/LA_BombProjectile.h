// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Projectile/LA_Projectile.h"
#include "LA_BombProjectile.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_BombProjectile : public ALA_Projectile
{
    GENERATED_BODY()

public:
    ALA_BombProjectile();

protected:
    virtual void BeginPlay() override;

    // 폭발 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ExplosionRadius;

    // 폭발까지 걸리는 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FuseTime;

    FTimerHandle ExplosionTimerHandle;

    // 실제 터지는 로직
    void Explode();

    // 폭발 이펙트
    UPROPERTY(EditAnywhere, Category = "Effects")
    class UParticleSystem* ExplosionEffect;

    UPROPERTY(EditAnywhere, Category = "Effects")
    class USoundBase* ExplosionSound;

    // 폭탄은 직접 부딪혀서 터지기보다 타이머로 터지는 경우가 많으므로 오버라이드해서 비워둡니다.
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
