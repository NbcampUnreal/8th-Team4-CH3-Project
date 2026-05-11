// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/LA_RecoveryShower.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Character/Player/Component/LA_HealthComponent.h"

// Sets default values
ALA_RecoveryShower::ALA_RecoveryShower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(FName("DefaultSceneRootComponent"));
    SetRootComponent(DefaultSceneRoot);

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(DefaultSceneRoot);

    Collision = CreateDefaultSubobject<USphereComponent>(FName("SphereShapeComponent"));
    Collision->SetupAttachment(StaticMeshComponent);

    InteractableIndicator = CreateDefaultSubobject<UWidgetComponent>(FName("InteractableIndicatorUI"));
    InteractableIndicator->SetupAttachment(DefaultSceneRoot);
    InteractableIndicator->SetWidgetSpace(EWidgetSpace::Screen);
}

// Called when the game starts or when spawned
void ALA_RecoveryShower::BeginPlay()
{
	Super::BeginPlay();

    // 초기에는 UI가 보이지 않도록 설정
    InteractableIndicator->SetVisibility(false);

    Collision->OnComponentBeginOverlap.AddDynamic(this, &ALA_RecoveryShower::OnBeginOverlap);
    Collision->OnComponentEndOverlap.AddDynamic(this, &ALA_RecoveryShower::OnEndOverlap);
}

// Called every frame
void ALA_RecoveryShower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALA_RecoveryShower::Interact_Implementation(AActor* InteractInstigator)
{
    // 상호작용 지시자에 대한 nullptr 방어
    if (InteractInstigator == nullptr)
    {
        return;
    }

    if (AdjacentActors.Contains(InteractInstigator) == false)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Too Far to take heal"));
        return;
    }

    // HealthComponent 확인
    ULA_HealthComponent* HealthComponent = InteractInstigator->FindComponentByClass<ULA_HealthComponent>();
    if (HealthComponent != nullptr && RecoveryAmount > 0)
    {
        // 체력 회복
        HealthComponent->Heal(RecoveryAmount);
    }
}
void ALA_RecoveryShower::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->Controller == nullptr)
        {
            return;
        }

        if (Character->Controller->IsLocalPlayerController() == true)
        {
            // 상호작용 안내 UI 활성화
            InteractableIndicator->SetVisibility(true);

            // 인접한 액터 목록에 추가
            AdjacentActors.Add(OtherActor);
        }
    }
}

void ALA_RecoveryShower::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->Controller == nullptr)
        {
            return;
        }

        if (Character->Controller->IsLocalPlayerController() == true)
        {
            // 상호작용 안내 UI 비 활성화
            InteractableIndicator->SetVisibility(false);

            // 인접한 액터 목록에서 제거
            AdjacentActors.Remove(OtherActor);
        }
    }
}

