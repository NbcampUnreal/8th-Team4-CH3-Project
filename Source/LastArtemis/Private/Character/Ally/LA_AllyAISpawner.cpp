// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_AllyAISpawner.h"

#include "Components/ArrowComponent.h"
#include "Character/Ally/LA_AllyAI.h"
#include "GameMode/LA_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALA_AllyAISpawner::ALA_AllyAISpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Scene = CreateDefaultSubobject<USceneComponent>("SceneComponent");
    SetRootComponent(Scene);

    SpawnDirection = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
    SpawnDirection->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALA_AllyAISpawner::BeginPlay()
{
	Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World || !AllyClass) return;

    // 자기 자신의 위치와 회전값 저장
    FVector Location = GetActorLocation();
    FRotator Rotation = GetActorRotation();

    // 스폰 파라미터
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 아군 소환
    ALA_AllyAI* SpawnedAlly = World->SpawnActor<ALA_AllyAI>(AllyClass, Location, Rotation, SpawnParams);

    if (SpawnedAlly)
    {
        // 스폰시 애니메이션 초기화
        if (USkeletalMeshComponent* SkeletalMeshComp = SpawnedAlly->GetMesh())
        {
            // 일시정지 해제
            SkeletalMeshComp->SetComponentTickEnabled(true);
            // 애니메이션 블루프린트 강제 재초기화
            SkeletalMeshComp->InitializeAnimScriptInstance(true);
            // 본 포즈 리프레시
            SkeletalMeshComp->RefreshBoneTransforms();

        }

        // 게임모드에 SpawnedAlly 저장
        if (AGameModeBase* GM = UGameplayStatics::GetGameMode(World))
        {
            if (ALA_GameModeBase* MyGM = Cast<ALA_GameModeBase>(GM))
            {
                // Todo : 게임모드에 스폰된 AllyAI 저장하는 함수 만들기
                // void RegisterAlly(ALA_AllyAI* NewAlly) { SpawnedAllies.Add(NewAlly); }
                // TArray<ALA_AllyAI*> SpawnedAllies;


                // MyGM->RegisterAlly(SpawnedAlly);
            }
        }
    }
}



