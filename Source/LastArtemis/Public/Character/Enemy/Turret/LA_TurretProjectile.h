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
    // Sets default values for this actor's properties
    ALA_TurretProjectile();

protected:
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
