// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_AllyAISpawner.h"

#include "Components/ArrowComponent.h"
#include "Character/Ally/LA_AllyAI.h"
#include "Components/CapsuleComponent.h"
#include "GameMode/LA_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALA_AllyAISpawner::ALA_AllyAISpawner()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* SceneComp = CreateDefaultSubobject<USceneComponent>(FName("SceneComponent"));
    SetRootComponent(SceneComp);
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CapsuleComponent"));
    CapsuleComp->SetupAttachment(SceneComp);
    CapsuleComp->InitCapsuleSize(87.406487f, 30.018118f);

    SpawnDirection = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
    SpawnDirection->SetupAttachment(RootComponent);
}

// 게임 시작 시 bSpawnOnBeginPlay이 true일 경우,
// 플레이어 주변에 Ally 자동 생성
void ALA_AllyAISpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnOnBeginPlay)
    {
        APawn* PlayerPawn = GetWorld() && GetWorld()->GetFirstPlayerController()
            ? GetWorld()->GetFirstPlayerController()->GetPawn()
            : nullptr;

        RespawnAlliesNearPlayer(PlayerPawn);
    }
}

// 체크 포인트 상호작용 시 Ally를 플레이어 뒤쪽으로 재배치
// 기존 Ally 재사용하는 방식으로 수정
void ALA_AllyAISpawner::RespawnAlliesNearPlayer(APawn* PlayerPawn)
{
    if (!PlayerPawn)
        return;

    if (!AllyClass)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    if (SpawnedAllies.Num() < AllySpawnCount)
    {
        SpawnedAllies.SetNum(AllySpawnCount);
    }

    // 새 Ally 스폰 시 설정 
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = PlayerPawn;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    for (int32 Index = 0; Index < AllySpawnCount; ++Index)
    {
        const FVector SpawnLocation = GetAllySpawnLocation(PlayerPawn, Index);
        const FRotator SpawnRotation = PlayerPawn->GetActorRotation();

        // 이미 Ally가 존재할 경우 재사용
        if (IsValid(SpawnedAllies[Index]))
        {
            SpawnedAllies[Index]->ReviveAtLocation(SpawnLocation, SpawnRotation);
            continue;
        }

        // Ally 새로 생성할 때 스폰
        ALA_AllyAI* NewAlly = World->SpawnActor<ALA_AllyAI>(
            AllyClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (!NewAlly)
            continue;

        SpawnedAllies[Index] = NewAlly;
    }
}

// 생성할 Ally의 위치 계산 
FVector ALA_AllyAISpawner::GetAllySpawnLocation(APawn* PlayerPawn, int32 AllyIndex) const
{
    if (!PlayerPawn)
    {
        return GetActorLocation();
    }

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const FVector Forward = PlayerPawn->GetActorForwardVector();
    const FVector Right = PlayerPawn->GetActorRightVector();

    // 오른쪽, 왼쪽에 하나씩 배치
    const float SideSign = AllyIndex % 2 == 0 ? -1.0f : 1.0f;

    return PlayerLocation - Forward * SpawnBackDistance + Right * SpawnSideDistance * SideSign;
}
