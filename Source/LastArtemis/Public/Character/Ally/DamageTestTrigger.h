// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageTestTrigger.generated.h"

UCLASS()
class LASTARTEMIS_API ADamageTestTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADamageTestTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Component")
    class UBoxComponent* BoxComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destroy Setting")
    bool bDestroyOnHit = false;

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageAmount;
};
