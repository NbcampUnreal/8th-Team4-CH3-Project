// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "LA_BossController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class ALA_Projectile;

UCLASS()
class LASTARTEMIS_API ALA_BossController : public AAIController
{
    GENERATED_BODY()

public:
    ALA_BossController();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Combat")
    TSubclassOf<class ALA_Projectile> BombClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* BossPerceptionComponent;

    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* TargetPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Settings", meta = (ClampMin = "500.0", UIMin = "500.0"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Settings")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Settings")
    float LoseSightRadius;

    // 발사할 탄환 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Combat")
    TSubclassOf<ALA_Projectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Combat")
    int32 MaxBasicShots = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Combat")
    float BasicShotInterval = 0.5f;

    int32 CurrentShotsFired;
    FTimerHandle MultiShotTimerHandle;

    UFUNCTION()
    void ShootSingleShot();

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void RotateToTarget();

    UFUNCTION()
    void ExecutePhasePattern();

    UFUNCTION()
    void ShootBasic3Shot();

    UFUNCTION()
    void ShootFanPattern();

    UFUNCTION()
    void ShootBomb();

    FTimerHandle AttackTimerHandle;
    bool bIsAttacking;
};
