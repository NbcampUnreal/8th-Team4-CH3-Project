#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_WeaponData.h"
#include "LA_WeaponBase.generated.h"

//class ULA_WeaponData;

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
    virtual void Tick(float DeltaTime) override;

    virtual float GetDynamicSpreadAngle() const;

    virtual bool CanFire() const;
    virtual void Fire();
    virtual void HitScan();
    virtual void UpdateAmmo();

    virtual void ApplyRecoil();
    virtual void ResetRecoil();
    virtual void ResetState();

public:
    UFUNCTION(BlueprintCallable, Category = "Weapon|Data")
    void SetWeaponData(ULA_WeaponData* NewWeaponData);

    UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
    float Draw(bool bActivate);

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
    ULA_WeaponData* WeaponData;

private:
    float CameraPitch;
    float CameraYaw;
    float CurrentSpreadAngle;

    int32 CurrentMagazineAmmo;
    int32 CurrentSpareAmmo;

    FRotator TargetSway;
    FRotator CurrentSway;

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

    int32 GetCurrentMagazineAmmo() const { return CurrentMagazineAmmo; }
    int32 GetMaxMagazineSize() const { return WeaponData->MaxMagazineSize; }
};
