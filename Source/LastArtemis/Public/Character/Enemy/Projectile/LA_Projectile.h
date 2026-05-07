// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_Projectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class LASTARTEMIS_API ALA_Projectile : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALA_Projectile();

protected:
    // 물리 충돌 범위
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComp;

    // 투사체 외형
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ProjectileMesh;

    // 투사체 이동 로직 (속도, 중력 등)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovement;

    // 기본 스탯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage;

    // 자식에서 오버라이드할 수 있도록 virtual 키워드 사용
    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    virtual void BeginPlay() override;
};
