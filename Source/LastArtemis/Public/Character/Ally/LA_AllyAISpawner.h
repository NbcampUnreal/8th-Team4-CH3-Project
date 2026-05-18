// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_AllyAISpawner.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_AllyAISpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALA_AllyAISpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // 방향 확인용 애로우 컴포넌트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    class UArrowComponent* SpawnDirection;
    // 스폰할 AllyAI 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    TSubclassOf<class ALA_AllyAI> AllyClass;


};
