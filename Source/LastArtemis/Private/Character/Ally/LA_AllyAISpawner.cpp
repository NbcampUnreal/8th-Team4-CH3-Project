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

    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CapsuleComponent"));
    SetRootComponent(CapsuleComp);

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

// 기존에 스포너가 만든 Ally를 제거
// 플레이어 주변에 Ally를 새로 생성
void ALA_AllyAISpawner::RespawnAlliesNearPlayer(APawn* PlayerPawn)
{
    if (!PlayerPawn)
        return;

    if (!AllyClass)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    // 기존에 이 스포너가 만든 Ally 제거
    for (ALA_AllyAI* Ally : SpawnedAllies)
    {
        if (IsValid(Ally))
        {
            Ally->Destroy();
        }
    }

    SpawnedAllies.Empty();

    // 플레이어 뒤 쪽에 설정된 수만큼 Ally 생성
    for (int32 Index = 0; Index < AllySpawnCount; ++Index)
    {
        const FVector SpawnLocation = GetAllySpawnLocation(PlayerPawn, Index);
        const FRotator SpawnRotation = PlayerPawn->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = PlayerPawn;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ALA_AllyAI* NewAlly = World->SpawnActor<ALA_AllyAI>(
            AllyClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (NewAlly)
        {
            // 재스폰 시 제거할 수 있도록 생성된 Ally 저장
            SpawnedAllies.Add(NewAlly);

            // 스폰 직후 애니메이션 보정?
            if (USkeletalMeshComponent* SkeletalMeshComp = NewAlly->GetMesh())
            {
                SkeletalMeshComp->SetComponentTickEnabled(true);
                SkeletalMeshComp->InitializeAnimScriptInstance(true);
                SkeletalMeshComp->RefreshBoneTransforms();
            }
        }
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
