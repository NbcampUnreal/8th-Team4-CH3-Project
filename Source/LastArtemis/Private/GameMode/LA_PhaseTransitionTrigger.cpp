// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_PhaseTransitionTrigger.h"
#include "GameMode/LA_GameModeBase.h"
#include "GameMode/LA_GameStateBase.h"
#include "Character/Player/LA_PlayerCharacter.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALA_PhaseTransitionTrigger::ALA_PhaseTransitionTrigger()
    :
    TriggerBoxExtent(FVector(100.0f, 100.0f, 100.0f))
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(SceneComponent);

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(SceneComponent);

    // Pawn을 제외한 모든 충돌 무시
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);     
    TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);               
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);          
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    TriggerBox->SetBoxExtent(TriggerBoxExtent);
}

// Called when the game starts or when spawned
void ALA_PhaseTransitionTrigger::BeginPlay()
{
	Super::BeginPlay();

    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(
            this,
            &ALA_PhaseTransitionTrigger::OnTriggerBeginOverlap
        );
    }
}

// 플레이어와 Overlap할 시 다음 Phase로 이동
// 한번 충돌 하면 그 다음부터 충돌 감지 X
void ALA_PhaseTransitionTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (bTriggered)
        return;

    ALA_PlayerCharacter* LA_PlayerCharacter = Cast<ALA_PlayerCharacter>(OtherActor);
    if (!LA_PlayerCharacter)
        return;

    ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(
        UGameplayStatics::GetGameMode(GetWorld())
    );

    if (!LA_GameMode)
        return;

    ALA_GameStateBase* LA_GameState = LA_GameMode->GetLAGameState();
    if (!LA_GameState)
        return;

    if (!LA_GameState->IsCurrentPhaseCompleted())
        return;

    bTriggered = true;

    LA_GameMode->AdvanceToNextPhase();

    // 충돌 꺼버림
    if (TriggerBox)
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

