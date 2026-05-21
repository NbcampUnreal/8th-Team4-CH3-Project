#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LA_WeaponData.generated.h"

UCLASS(BlueprintType)
class LASTARTEMIS_API ULA_WeaponData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // AssetManager에서 생성된 DataAsset을 인식하기 위하여 함수 오버라이딩
    FORCEINLINE virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(WeaponType, GetFName()); }

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    FPrimaryAssetType WeaponType;

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
    USkeletalMesh* WeaponMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
    TSubclassOf<class UAnimInstance> WeaponAnimBP = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
    class UMaterialInterface* DecalMaterial;


    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    float BaseDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    float FireRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    float MaxRange = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 PelletCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 MaxMagazineSize = 30;


    // Aim
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    float DefaultFOV = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    float AimFOV = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    float AimDistanceOffset = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    float AimInterpSpeed = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    FVector DefaultMeshLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aim")
    FRotator DefaultMeshRotation = FRotator::ZeroRotator;


    // Spread
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float DefaultSpreadAngle = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float MaxSpreadAngle = 4.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float MinSpreadAngle = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float SpreadIncrement = 0.4f;


    // Recoil
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Recoil")
    class UCurveVector* RecoilCurve = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Recoil")
    float RecoilSpeed = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Recoil")
    float RecoilResetTime = 0.5f;


    // Sway
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Sway")
    float MaxSwayDegree = 4.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Sway")
    float SwayMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Sway")
    float SwaySpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Sway")
    float SwayReturnSpeed = 15.f;


    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
    class UAnimMontage* DrawMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
    class UAnimMontage* FireMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
    class UAnimMontage* ReloadMontage = nullptr;
};
