// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_CheckPointActor.h"
#include "GameMode/LA_GameInstance.h"
#include "GameMode/LA_GameStateBase.h"
#include "Character/LA_BaseCharacter.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Item/LA_InventoryComponent.h"
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

    InteractableIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractableIndicatorUI"));
    InteractableIndicator->SetupAttachment(SceneComponent);
    InteractableIndicator->SetWidgetSpace(EWidgetSpace::Screen);

    bPlayerInRange = false;
    OverlappingPlayerPawn = nullptr;

    RecoveryAmount = 500.0f;
    DecontaminationAmount = 100.0f;
}

// Called when the game starts or when spawned
void ALA_CheckPointActor::BeginPlay()
{
	Super::BeginPlay();

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(false);
    }

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

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(true);
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

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(false);
    }
}

// Check Point와 상호 작호한 위치 LA_GameInstance에 저장
// -> 위치 세이브 포인트로 사용
void ALA_CheckPointActor::InteractCheckPoint(APawn* InteractingPawn)
{
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

    // 세이브 데이터 저장
    const int32 CurrentPhaseIndex = LA_GameState->GetCurrentPhaseIndex();
    const FVector SaveLocation = InteractingPawn->GetActorLocation();
    const FRotator SaveRotation = InteractingPawn->GetActorRotation();
    const int32 SaveElapsedGameTime = LA_GameState->GetElapsedGameTime();
    const int32 CurrentScore = LA_GameInstance->GetScore();

    LA_GameInstance->SaveCheckPointData(CurrentPhaseIndex, SaveLocation, SaveRotation, SaveElapsedGameTime, CurrentScore);
    LA_GameInstance->SaveGameData();

    // 체력 회복 / 오염도 제거
    RecoverPlayer(InteractingPawn);
    Decontaminate(InteractingPawn);

    // 아이템 보급
    RefillItems(InteractingPawn);

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint Interacted - PhaseIndex: %d, Location: %s, ElapsedGmeTime: %d"),
        CurrentPhaseIndex,
        *SaveLocation.ToString(),
        SaveElapsedGameTime
    );
}

void ALA_CheckPointActor::RecoverPlayer(APawn* InteractingPawn)
{
    if (!InteractingPawn)
        return;

    ULA_HealthComponent* HealthComponent = InteractingPawn->FindComponentByClass<ULA_HealthComponent>();
    if (!HealthComponent)
        return;

    if (RecoveryAmount <= 0.0f)
        return;

    HealthComponent->Heal(RecoveryAmount);
}

void ALA_CheckPointActor::Decontaminate(APawn* InteractingPawn)
{
    if (!InteractingPawn)
        return;

    if (DecontaminationAmount <= 0.0f)
        return;

    ULA_HealthComponent* HealthComponent = InteractingPawn->FindComponentByClass<ULA_HealthComponent>();
    if (!HealthComponent)
        return;

    HealthComponent->Decontaminate(DecontaminationAmount);
}

void ALA_CheckPointActor::RefillItems(APawn* InteractingPawn)
{
    if (!InteractingPawn)
        return;

    ULA_InventoryComponent* InventoryComponent = InteractingPawn->FindComponentByClass<ULA_InventoryComponent>();

    if (!InventoryComponent)
        return;

    InventoryComponent->RefillItem(HealingItemData);
    InventoryComponent->RefillItem(DecontaminationItemData);

    InventoryComponent->PrintInventory();

    UE_LOG(LogTemp, Warning, TEXT("HealingItemData: %s"),
        HealingItemData ? *HealingItemData->GetName() : TEXT("nullptr"));

    UE_LOG(LogTemp, Warning, TEXT("DecontaminationItemData: %s"),
        DecontaminationItemData ? *DecontaminationItemData->GetName() : TEXT("nullptr"));
}

// 상호작용 시 체크 포인트 처리
void ALA_CheckPointActor::Interact_Implementation(AActor* InteractInstigator)
{
    APawn* InteractingPawn = Cast<APawn>(InteractInstigator);
    if (!InteractingPawn)
        return;

    // 세이브 포인트 저장 및 체력 회복
    InteractCheckPoint(InteractingPawn);
}
