// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_EnemySpawner.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALA_EnemySpawner();

protected:
    virtual void BeginPlay() override;

    // 실제로 몬스터를 스폰하는 함수
    void SpawnEnemy();

    // 일정 시간마다 SpawnEnemy를 호출해줄 타이머 핸들
    FTimerHandle SpawnTimerHandle;

    // 현재 이 스포너가 살려놓은 에너미 수
    int32 CurrentSpawnCount = 0;

    int32 TotalSpawnedCount = 0;

    // 에너미가 죽었을 때 스포너에게 알려주어 카운트를 줄이기 위한 함수
    UFUNCTION()
    void OnEnemyDestroyed(AActor* DestroyedActor);

    // 1. 어떤 에너미를 스폰할 것인가? (BP_LA_Enemy 같은 블루프린트 클래스 선택용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Settings")
    TSubclassOf<ACharacter> EnemyClassToSpawn;

    // 2. 몇 초마다 스폰할 것인가?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Settings")
    float SpawnInterval = 5.0f;

    // 3. 이 스포너에서 동시에 존재할 수 있는 최대 에너미 수는?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Settings")
    int32 MaxEnemyCount = 3;

    // 4. 스폰할 반경 (스포너 위치 기준 반지름 cm)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Settings")
    float SpawnRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Settings")
    int32 TotalMonstersToSpawn = 5;

protected:
    // 클리어한 Phase인지
    bool CanSpawnByPhase() const;

    // 특정 Phase 진입 시 몬스터 생성
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase Spawn")
    int32 SpawnBeforePhaseIndex = 0;
};
