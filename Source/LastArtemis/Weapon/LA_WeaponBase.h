#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chaos/ChaosEngineInterface.h"
#include "LA_WeaponData.h"
#include "LA_WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAimStateChanged, bool, bIsAiming);

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Draw,
    Idle,
    Fire,
    Reload
};

UCLASS()
class LASTARTEMIS_API ALA_WeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ALA_WeaponBase();

protected:
    virtual void Tick(float DeltaTime) override;

    virtual void Fire();
    virtual void HitScan();
    virtual void ApplyRecoil();
    virtual void ResetRecoil();
    virtual void ResetState();

public:
    UFUNCTION(BlueprintCallable, Category = "Weapon|Data")
    void SetWeaponData(ULA_WeaponData* NewWeaponData);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    void Draw();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    void Look(FVector InputValue);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StartAiming();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StopAiming();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    virtual void Reload();

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnAimStateChanged OnAimStateChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
    EWeaponState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
    int32 CurrentMagazineAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
    TMap<TEnumAsByte<EPhysicalSurface>, class UNiagaraSystem*> ImpactParticles;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    USkeletalMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
    ULA_WeaponData* WeaponData;

private:
    float CameraPitch;
    float CameraYaw;
    float CurrentSpreadAngle;
    int32 CurrentSpareAmmo;

    FRotator TargetSway;
    FRotator CurrentSway;

    bool bIsAiming;
    FVector AimMeshLocation;
    FRotator AimMeshRotation;

    int32 CurrentShotCount;
    FRotator TargetRecoil;
    FRotator CurrentRecoil;

    FTimerHandle FireTimerHandle;
    FTimerHandle StateTimerHandle;
    FTimerHandle RecoilResetTimerHandle;

public:
    UFUNCTION(BlueprintPure, Category = "Weapon|Components")
    class UCameraComponent* GetFirstPersonCamera() const { return Camera; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Action")
    bool IsAiming() const { return bIsAiming; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Spread")
    float GetCurrentSpreadAngle() const { return CurrentSpreadAngle; }

    int32 GetCurrentMagazineAmmo() const { return CurrentMagazineAmmo; }
    int32 GetMaxMagazineSize() const { return WeaponData->MaxMagazineSize; }

    void UpdateAmmo();
    void RefillAmmo();
};
