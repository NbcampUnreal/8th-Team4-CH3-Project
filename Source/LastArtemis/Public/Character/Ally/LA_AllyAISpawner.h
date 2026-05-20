// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_AllyAISpawner.generated.h"

class UCapsuleComponent;
class UArrowComponent;
class ALA_AllyAI;
class APawn;

UCLASS(BlueprintType)
class LASTARTEMIS_API ALA_AllyAISpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALA_AllyAISpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    TObjectPtr<UCapsuleComponent> CapsuleComp;
    // 방향 확인용 애로우 컴포넌트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    TObjectPtr<UArrowComponent> SpawnDirection;


public:
    // 플레이어 근처로 리스폰
    UFUNCTION(BlueprintCallable, Category = "Ally Spawn")
    void RespawnAlliesNearPlayer(APawn* PlayerPawn);

    // AllAI 위치 반환
    FVector GetAllySpawnLocation(APawn* PlayerPawn, int32 AllyIndex) const;

protected:
    // 스폰할 AllyAI 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    TSubclassOf<ALA_AllyAI> AllyClass;

    // 최대 스폰 카운트
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    int32 AllySpawnCount = 2;

    // 체크 포인트에서 스폰될 위치
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    float SpawnBackDistance = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    float SpawnSideDistance = 50.0f;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    bool bSpawnOnBeginPlay = false;

    // 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ally Spawn")
    TArray<TObjectPtr<ALA_AllyAI>> SpawnedAllies;


};
