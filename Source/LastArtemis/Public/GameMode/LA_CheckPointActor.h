// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_CheckPointActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ALA_PlayerCharacter;
class APawn;

UCLASS(BlueprintType)
class LASTARTEMIS_API ALA_CheckPointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALA_CheckPointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
    // Check Point 범위 안으로 들어올 때 상호작용 가능하도록
    UFUNCTION()
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

public:
    // CheckPoint와 상호작용
    // 세이브, 체력 회복 등등
    UFUNCTION(BlueprintCallable, Category = "Check Point")
    void InteractCheckPoint(APawn* InteractingPawn);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    // 플레이어가 상호 작용 가능한 범위 안에 있는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Check Point")
    bool bPlayerInRange;

    // 현재 범위 안에 있는 플레이어
    UPROPERTY()
    APawn* OverlappingPlayerPawn;

};
