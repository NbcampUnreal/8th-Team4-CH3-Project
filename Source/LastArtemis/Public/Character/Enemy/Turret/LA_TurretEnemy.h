// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_TurretEnemy.generated.h"


UCLASS()
class LASTARTEMIS_API ALA_TurretEnemy : public ALA_BaseCharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ALA_TurretEnemy();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* DeathExplosionEffect;

    // 포탑 헤드 (회전할 부분)
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TurretHead;

    // 사거리 및 회전 속도
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DetectionRange;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed;

    // 발사 관련
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ALA_Projectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float FireRate;

    FTimerHandle FireTimerHandle;

    // 타겟 탐색 및 공격 로직
    void FindTarget();
    void FireProjectile();
    bool CheckLineOfSight(AActor* TargetActor);
    void SwitchTeam(FGameplayTag NewTeamTag);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

    UPROPERTY()
    AActor* CurrentTarget;
};
