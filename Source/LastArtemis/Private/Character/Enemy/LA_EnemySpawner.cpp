// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/LA_EnemySpawner.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ALA_EnemySpawner::ALA_EnemySpawner()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALA_EnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 게임이 시작되면 SpawnInterval마다 주기적으로 SpawnEnemy 함수를 반복 실행합니다.
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ALA_EnemySpawner::SpawnEnemy, SpawnInterval, true);
}


void ALA_EnemySpawner::SpawnEnemy()
{
    // 예외처리: 스폰할 에너미 클래스가 지정 안 됐거나, 이미 최대 마리수라면 스킵
    if (!EnemyClassToSpawn || TotalSpawnedCount >= TotalMonstersToSpawn)
    {
        // 5마리 다 소환했으니 타이머를 꺼서 더 이상 이 함수가 안 돌게 만듭니다.
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    if (GetWorld())
    {
        FVector SpawnLocation = GetActorLocation() + UKismetMathLibrary::RandomPointInBoundingBox(FVector::ZeroVector, FVector(SpawnRadius, SpawnRadius, 0.0f));
        FRotator SpawnRotation = GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(EnemyClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedEnemy)
        {
            CurrentSpawnCount++; // 현재 살아있는 수 증가
            TotalSpawnedCount++;  //중요: 소환된 총 누적 마릿수 증가!

            SpawnedEnemy->OnDestroyed.AddDynamic(this, &ALA_EnemySpawner::OnEnemyDestroyed);
        }
    }
}

void ALA_EnemySpawner::OnEnemyDestroyed(AActor* DestroyedActor)
{
    CurrentSpawnCount = FMath::Max(0, CurrentSpawnCount - 1);

    // 핵심: 5마리를 다 소환했고(`TotalSpawnedCount >= TotalMonstersToSpawn`),
    // 필드에 살아있는 몬스터마저 0마리가 되었다면? -> "방 클리어!"
    if (TotalSpawnedCount >= TotalMonstersToSpawn && CurrentSpawnCount == 0)
    {
        Destroy();
    }
}



