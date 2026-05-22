// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LA_PhaseLockedDoor.h"
#include "GameMode/LA_GameStateBase.h"
#include "Character/Player/LA_PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

// Sets default values
ALA_PhaseLockedDoor::ALA_PhaseLockedDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    SetRootComponent(SceneComponent);

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(SceneComponent);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(SceneComponent);
    InteractionSphere->SetSphereRadius(200.0f);

    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    InteractableIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractableIndicatorUI"));
    InteractableIndicator->SetupAttachment(SceneComponent);
    InteractableIndicator->SetWidgetSpace(EWidgetSpace::Screen);

    bPlayerInRange = false;
    OverlappingPlayerPawn = nullptr;

    bOpened = false;
    bOpening = false;

    RequiredClearedPhaseIndex = 0;

    OpenLocationOffset = FVector(0.0f, 0.0f, 300.0f);

    OpenInterpSpeed = 170.0f;

    LockedMessageDisplayTime = 2.0f;
}

// Called when the game starts or when spawned
void ALA_PhaseLockedDoor::BeginPlay()
{
	Super::BeginPlay();

    if (StaticMeshComponent)
    {
        ClosedDoorLocation = StaticMeshComponent->GetRelativeLocation();
        OpenedDoorLocation = ClosedDoorLocation + OpenLocationOffset;
    }

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(false);
    }


    // 범위 안에 들어오면 바인딩 
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(
            this,
            &ALA_PhaseLockedDoor::OnInteractionBeginOverlap
        );

        InteractionSphere->OnComponentEndOverlap.AddDynamic(
            this,
            &ALA_PhaseLockedDoor::OnInteractionEndOverlap
        );
    }
 
    //GetWorldTimerManager().SetTimerForNextTick(this, &ALA_PhaseLockedDoor::ApplyInitialDoorState);
}

// Called every frame
void ALA_PhaseLockedDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!bOpening || !StaticMeshComponent)
        return;

    const FVector CurrentLocation = StaticMeshComponent->GetRelativeLocation();
    // 보간 이동
    const FVector NewLocation = FMath::VInterpConstantTo(
        CurrentLocation,
        OpenedDoorLocation,
        DeltaTime,
        OpenInterpSpeed
    );

    StaticMeshComponent->SetRelativeLocation(NewLocation);

    // 위치 보정
    if (FVector::DistSquared(NewLocation, OpenedDoorLocation) <= 1.0f)
    {
        StaticMeshComponent->SetRelativeLocation(OpenedDoorLocation);
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // 사운드 정지
        if (DoorOpeningAudioComponent)
        {
            DoorOpeningAudioComponent->Stop();
            DoorOpeningAudioComponent = nullptr;
        }

        bOpening = false;
        bOpened = true;
        SetActorTickEnabled(false);
    }
}

void ALA_PhaseLockedDoor::OnInteractionBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    ALA_PlayerCharacter* PlayerCharacter = Cast<ALA_PlayerCharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    if (bOpened || bOpening)
        return;

    bPlayerInRange = true;
    OverlappingPlayerPawn = PlayerCharacter;

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(true);
    }
}

void ALA_PhaseLockedDoor::OnInteractionEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    ALA_PlayerCharacter* PlayerCharacter = Cast<ALA_PlayerCharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    if (OverlappingPlayerPawn == PlayerCharacter)
    {
        bPlayerInRange = false;
        OverlappingPlayerPawn = nullptr;
    }

    HideLockedMessage();
    GetWorldTimerManager().ClearTimer(LockedMessageTimerHandle);

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(false);
    }
}

bool ALA_PhaseLockedDoor::CanOpenDoor() const
{
    ALA_GameStateBase* LA_GameState = GetWorld()->GetGameState<ALA_GameStateBase>();
    if (!LA_GameState)
        return false;

    const int32 CurrentPhaseIndex = LA_GameState->GetCurrentPhaseIndex();

    // 현재 Phase가 요구 Phase보다 크면
    // 이미 클리어한 것으로 간주
    if (CurrentPhaseIndex > RequiredClearedPhaseIndex)
    {
        return true;
    }

    // 현재 Phase가 요구 Phase이고
    // 현재 Phase가 완료된 상태
    if (
        CurrentPhaseIndex == RequiredClearedPhaseIndex &&
        LA_GameState->IsCurrentPhaseCompleted()
        )
    {
        return true;
    }

    return false;
}

void ALA_PhaseLockedDoor::OpenDoor()
{
    if (bOpened || bOpening)
        return;

    bOpening = true;
    // 문 이동 시작 시 tick 활성화
    SetActorTickEnabled(true);

    if (DoorOpeningLoopSound && StaticMeshComponent)
    {
        DoorOpeningAudioComponent = UGameplayStatics::SpawnSoundAttached(
            DoorOpeningLoopSound,
            StaticMeshComponent,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true,
            1.0f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            false
        );
    }

    HideLockedMessage();
    GetWorldTimerManager().ClearTimer(LockedMessageTimerHandle);

    if (InteractableIndicator)
    {
        InteractableIndicator->SetVisibility(false);
    }

    if (InteractionSphere)
    {
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ALA_PhaseLockedDoor::Interact_Implementation(AActor* InteractInstigator)
{
    APawn* InteractingPawn = Cast<APawn>(InteractInstigator);
    if (!InteractingPawn)
        return;

    // 플레이어가 입력한 상호 작용인지
    if (!bPlayerInRange || OverlappingPlayerPawn != InteractingPawn)
        return;

    if (bOpened || bOpening)
        return;

    // Phase 클리어 못하면 잠김 메시지 표시
    if (!CanOpenDoor())
    {
        ShowLockedMessage();

        if (DoorLockedSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DoorLockedSound, GetActorLocation());
        }

        return;
    }

    OpenDoor();
}

// 이미 클리어한 한 후,
// 게임을 다시 불러오기 할 경우
// 문 열린 상태로 설정
void ALA_PhaseLockedDoor::ApplyInitialDoorState()
{
    //if (!CanOpenDoor())
    //    return;

    //bOpened = true;
    //bOpening = false;

    //if (StaticMeshComponent)
    //{
    //    StaticMeshComponent->SetRelativeLocation(OpenedDoorLocation);
    //    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //}

    //if (InteractableIndicator)
    //{
    //    InteractableIndicator->SetVisibility(false);
    //}

    //if (InteractionSphere)
    //{
    //    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //}

    //SetActorTickEnabled(false);
}

void ALA_PhaseLockedDoor::ShowLockedMessage()
{
    if (!InteractableIndicator)
        return;

    InteractableIndicator->SetVisibility(true);

    UUserWidget* IndicatorWidget = InteractableIndicator->GetUserWidgetObject();
    if (!IndicatorWidget)
        return;
    UTextBlock* InteractKeyText = Cast<UTextBlock>(IndicatorWidget->GetWidgetFromName(TEXT("InteractKeyText")));
    UTextBlock* LockedMessageText = Cast<UTextBlock>(IndicatorWidget->GetWidgetFromName(TEXT("LockedMessageText")));

    if (!InteractKeyText)
        return;
    if (!LockedMessageText)
        return;

    InteractKeyText->SetVisibility(ESlateVisibility::Collapsed);
    LockedMessageText->SetVisibility(ESlateVisibility::Visible);

    GetWorldTimerManager().ClearTimer(LockedMessageTimerHandle);

    GetWorldTimerManager().SetTimer(
        LockedMessageTimerHandle,
        this,
        &ALA_PhaseLockedDoor::HideLockedMessage,
        LockedMessageDisplayTime,
        false
    );
}

void ALA_PhaseLockedDoor::HideLockedMessage()
{
    if (!InteractableIndicator)
        return;

    UUserWidget* IndicatorWidget = InteractableIndicator->GetUserWidgetObject();
    if (!IndicatorWidget)
        return;

    UTextBlock* InteractKeyText = Cast<UTextBlock>(
        IndicatorWidget->GetWidgetFromName(TEXT("InteractKeyText"))
    );

    UTextBlock* LockedMessageText = Cast<UTextBlock>(
        IndicatorWidget->GetWidgetFromName(TEXT("LockedMessageText"))
    );

    if (LockedMessageText)
    {
        LockedMessageText->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (InteractKeyText)
    {
        InteractKeyText->SetVisibility(ESlateVisibility::Visible);
    }
}
