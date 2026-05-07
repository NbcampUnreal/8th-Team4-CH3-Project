// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/LA_TrapBase.h"

#include "GameplayTagAssetInterface.h"
#include "Components/BoxComponent.h"


// Sets default values
ALA_TrapBase::ALA_TrapBase()
{
    TrapArea = CreateDefaultSubobject<UBoxComponent>(TEXT("TrapArea"));
    RootComponent = TrapArea;

    // 기본 설정
    TickInterval = 0.5f;
}
void ALA_TrapBase::BeginPlay()
{
    Super::BeginPlay();
    TrapArea->OnComponentBeginOverlap.AddDynamic(this, &ALA_TrapBase::OnOverlapBegin);
    TrapArea->OnComponentEndOverlap.AddDynamic(this, &ALA_TrapBase::OnOverlapEnd);
}

void ALA_TrapBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Gameplay Tag 인터페이스를 사용하는 액터인지 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TagInterface)
    {
        FGameplayTagContainer OwnedTags;
        TagInterface->GetOwnedGameplayTags(OwnedTags);

        //  플레이어 측(Team.Ally) 태그를 가지고 있는지 확인
        FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));
        if (OwnedTags.HasTag(AllyTag))
        {
            OverlappingActors.AddUnique(OtherActor);

            if (!GetWorld()->GetTimerManager().IsTimerActive(TrapTickTimerHandle))
            {
                GetWorld()->GetTimerManager().SetTimer(TrapTickTimerHandle, this, &ALA_TrapBase::OnTrapTick, TickInterval, true);
            }

            UE_LOG(LogTemp, Log, TEXT("Ally (Player) detected by Gameplay Tag!"));
        }
    }
}

void ALA_TrapBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 나갈 때는 목록에 있는지 확인 후 제거 (태그 체크 없이 바로 제거해도 무방)
    if (OtherActor && OverlappingActors.Contains(OtherActor))
    {
        OverlappingActors.Remove(OtherActor);
    }

    if (OverlappingActors.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TrapTickTimerHandle);
    }
}

void ALA_TrapBase::OnTrapTick()
{
    for (AActor* Actor : OverlappingActors)
    {
        ApplyEffect(Actor);
    }
}


