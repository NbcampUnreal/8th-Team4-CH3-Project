// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_JumpPad.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_JumpPad : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALA_JumpPad();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* JumpArea;

    // 발사 강도
    UPROPERTY(EditAnywhere, Category = "Settings")
    float LaunchStrength;

    // 발사 방향 (기본은 위쪽)
    UPROPERTY(EditAnywhere, Category = "Settings")
    FVector LaunchDirection;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
