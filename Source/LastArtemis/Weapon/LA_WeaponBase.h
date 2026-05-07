#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Idle,
    Firing,
    Reloading
};

UCLASS()
class LASTARTEMIS_API ALA_WeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ALA_WeaponBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual float GetDynamicSpreadAngle() const;
    virtual bool CanFire() const;
    virtual void Fire();
    virtual void ResetState();

public:
    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    void Look(FVector InputValue);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void Reload();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    bool bIsAiming;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    USkeletalMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
    EWeaponState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimMontage* FiringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimMontage* ReloadMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway")
    float MaxSwayDegree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway")
    float SwayMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway")
    float SwaySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway")
    float SwayReturnSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    float DefaultFOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    float AimFOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    float AimDistanceOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    FVector DefaultMeshLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    FRotator DefaultMeshRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    FVector AimMeshLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    FRotator AimMeshRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
    float AimInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float FireRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float MaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    int32 PelletCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
    float DefaultSpreadAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
    float MaxSpreadAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
    float MinSpreadAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
    float SpreadIncrement;

public:
    UFUNCTION(BlueprintPure, Category = "Weapon|Components")
    class UCameraComponent* GetFirstPersonCamera() const { return Camera; }

private:
    float CameraPitch;
    float CameraYaw;
    float CurrentSpreadAngle;

    FRotator TargetSway;
    FRotator CurrentSway;

    FTimerHandle FireTimerHandle;
    FTimerHandle StateTimerHandle;
};
