// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_PhaseTransitionTrigger.generated.h"

class USceneComponent;
class UBoxComponent;

UCLASS(Blueprintable)
class LASTARTEMIS_API ALA_PhaseTransitionTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALA_PhaseTransitionTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    // 재사용 방지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phase")
    bool bTriggered = false;

    UFUNCTION()
    void OnTriggerBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    FVector TirggerBoxExtent;
};
