// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_CheckPointActor.h"
#include "GameMode/LA_GameInstance.h"
#include "GameMode/LA_GameStateBase.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"

// Sets default values
ALA_CheckPointActor::ALA_CheckPointActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(SceneComponent);

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(SceneComponent);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(SceneComponent);
    InteractionSphere->SetSphereRadius(100.0f);

    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    bPlayerInRange = false;
    OverlappingPlayerPawn = nullptr;

    bIntreactiveSavePoint = false;
}

// Called when the game starts or when spawned
void ALA_CheckPointActor::BeginPlay()
{
	Super::BeginPlay();

    // 범위 안에 들어오면 바인딩 
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(
            this,
            &ALA_CheckPointActor::OnInteractionBeginOverlap
        );

        InteractionSphere->OnComponentEndOverlap.AddDynamic(
            this,
            &ALA_CheckPointActor::OnInteractionEndOverlap
        );
    }
}


////////////////////////
/// Overlapping
////////////////////////

// Check Point 범위 안에 들어오면 UI 표시
void ALA_CheckPointActor::OnInteractionBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    ALA_PlayerCharacter* LA_PlayerCharacter = Cast<ALA_PlayerCharacter>(OtherActor);
    if (!LA_PlayerCharacter)
        return;

    bPlayerInRange = true;
    OverlappingPlayerPawn = LA_PlayerCharacter;

    // 상호 작용 가능 UI 표시
    {
        // F 입력 시 정비
        bIntreactiveSavePoint = true;
        InteractCheckPoint(LA_PlayerCharacter);
    }
}

void ALA_CheckPointActor::OnInteractionEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    ALA_PlayerCharacter* LA_PlayerCharacter = Cast<ALA_PlayerCharacter>(OtherActor);
    if (!LA_PlayerCharacter)
        return;

    if (OverlappingPlayerPawn == LA_PlayerCharacter)
    {
        bPlayerInRange = false;
        OverlappingPlayerPawn = nullptr;
    }

    // 상호 작용 가능 UI 숨김
    {
        bIntreactiveSavePoint = false;
    }
}

// Check Point와 상호 작호한 위치 LA_GameInstance에 저장
// -> 위치 세이브 포인트로 사용
void ALA_CheckPointActor::InteractCheckPoint(APawn* InteractingPawn)
{
    if (!bIntreactiveSavePoint)
        return;

    if (!InteractingPawn)
        return;

    if (!bPlayerInRange || OverlappingPlayerPawn != InteractingPawn)
        return;

    ULA_GameInstance* LA_GameInstance = GetGameInstance<ULA_GameInstance>();
    if (!LA_GameInstance)
        return;

    ALA_GameStateBase* LA_GameState = GetWorld()->GetGameState<ALA_GameStateBase>();
    if (!LA_GameState || LA_GameState->GetCurrentPhaseType() != ELA_PhaseType::Rest)
        return;

    const int32 CurrentPhaseIndex = LA_GameState->GetCurrentPhaseIndex();
    const FVector SaveLocation = InteractingPawn->GetActorLocation();
    const FRotator SaveRotation = InteractingPawn->GetActorRotation();

    LA_GameInstance->SaveCheckPointData(CurrentPhaseIndex, SaveLocation, SaveRotation);
    LA_GameInstance->SaveGameData();

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint Interacted - PhaseIndex: %d, Location: %s"),
        CurrentPhaseIndex,
        *SaveLocation.ToString()
    );
}
