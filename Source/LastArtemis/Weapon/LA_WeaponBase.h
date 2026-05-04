#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_WeaponBase.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_WeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ALA_WeaponBase();

protected:
	virtual void BeginPlay() override;

	virtual void Fire();

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	virtual void Reload();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	int32 PelletCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float SpreadAngle;

	FTimerHandle FireTimerHandle;
	bool bCanFire;
};
