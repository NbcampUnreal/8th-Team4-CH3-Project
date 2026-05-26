#include "Character/Enemy/LA_EnemySpawner.h"

#include "GameFramework/Character.h"
#include "GameMode/LA_GameStateBase.h"

ALA_EnemySpawner::ALA_EnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALA_EnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 현재 GameState 정보 가져옴
    ALA_GameStateBase* LA_GameState = GetWorld()
        ? GetWorld()->GetGameState<ALA_GameStateBase>()
        : nullptr;

    if (!LA_GameState)
        return;

    // Phase 바뀌면 HandleMissionStatusChanged 호출
    LA_GameState->OnMissionStatusChanged.AddDynamic(
        this,
        &ALA_EnemySpawner::HandleMissionStatusChanged
    );

    TrySpawn(LA_GameState->GetCurrentPhaseIndex());
}

void ALA_EnemySpawner::HandleMissionStatusChanged(
    ULA_MissionDataAsset* MissionData,
    int32 PhaseIndex,
    int32 CurrentCount
)
{
    TrySpawn(PhaseIndex);
}

void ALA_EnemySpawner::TrySpawn(int32 PhaseIndex)
{
    if (bHasSpawned)
        return;

    if (PhaseIndex != SpawnPhaseIndex)
        return;

    SpawnEnemy();
}

void ALA_EnemySpawner::SpawnEnemy()
{
    if (bHasSpawned)
        return;

    if (!EnemyClassToSpawn || !GetWorld())
        return;

    // 스폰 위치에 다른 액터가 있으면 위치 조정하여 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 몬스터 한 마리 스폰
    ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(
        EnemyClassToSpawn,
        GetActorLocation(),
        GetActorRotation(),
        SpawnParams
    );

    if (!SpawnedEnemy)
        return;

    // 재스폰 방지
    bHasSpawned = true;
}
