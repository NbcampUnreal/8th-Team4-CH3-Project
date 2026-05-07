// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/LA_JumpPad.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"


// Sets default values
ALA_JumpPad::ALA_JumpPad()
{
    JumpArea = CreateDefaultSubobject<UBoxComponent>(TEXT("JumpArea"));
    RootComponent = JumpArea;

    LaunchStrength = 1500.0f;
    LaunchDirection = FVector(0.0f, 0.0f, 1.0f);
}

void ALA_JumpPad::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        // 방향 * 강도 로 발사 벡터 계산
        FVector LaunchVelocity = LaunchDirection.GetSafeNormal() * LaunchStrength;

        // Z축은 확실하게 밀어주기 위해 bXYOverride는 false, bZOverride는 true 권장
        Character->LaunchCharacter(LaunchVelocity, false, true);

        UE_LOG(LogTemp, Log, TEXT("JumpPad Launched: %s"), *Character->GetName());
    }
}



