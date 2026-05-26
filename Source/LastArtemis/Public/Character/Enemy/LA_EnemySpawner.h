#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LA_EnemySpawner.generated.h"

class ACharacter;
class ULA_MissionDataAsset;

UCLASS(Blueprintable)
class LASTARTEMIS_API ALA_EnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    ALA_EnemySpawner();

protected:
    virtual void BeginPlay() override;

    // Phase 바뀌면 호출
    UFUNCTION()
    void HandleMissionStatusChanged(
        ULA_MissionDataAsset* MissionData,
        int32 PhaseIndex,
        int32 CurrentCount
    );

    // 현재 Phase와 스포너의 SpawnPhaseIndex 같으면 스폰 시도
    void TrySpawn(int32 PhaseIndex);
    void SpawnEnemy();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    TSubclassOf<ACharacter> EnemyClassToSpawn;

    // 이 Phase Index가 되었을 때 스폰
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    int32 SpawnPhaseIndex = 1;

    // 이미 스폰했는지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
    bool bHasSpawned = false;
};
