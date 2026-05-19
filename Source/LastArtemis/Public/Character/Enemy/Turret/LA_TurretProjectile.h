// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Projectile/LA_Projectile.h"
#include "LA_TurretProjectile.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_TurretProjectile : public ALA_Projectile
{
    GENERATED_BODY()

public:
    ALA_TurretProjectile();

protected:
    virtual void BeginPlay() override;

    // 🎯 부모의 OnOverlapBegin을 완벽하게 덮어쓰기 위해 virtual과 override를 정확히 명시합니다.
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
