// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/DamageTestTrigger.h"

#include "Character/Player/LA_PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

class ALA_BaseCharacter;
// Sets default values
ADamageTestTrigger::ADamageTestTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
    SetRootComponent(BoxComp);

    BoxComp->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    BoxComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

}

// Called when the game starts or when spawned
void ADamageTestTrigger::BeginPlay()
{
	Super::BeginPlay();
    BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ADamageTestTrigger::OnOverlapBegin);
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("DamageTestTrigger BeginPlay"));
}

void ADamageTestTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    if (!OtherActor || OtherActor == this) return;

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player || OtherActor != Player) return;

    FDamageEvent DamageEvent;
    Player->TakeDamage(
        DamageAmount,
        DamageEvent,
        nullptr,
        nullptr
    );

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Damage Triggered"));
}



// Called every frame
void ADamageTestTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

