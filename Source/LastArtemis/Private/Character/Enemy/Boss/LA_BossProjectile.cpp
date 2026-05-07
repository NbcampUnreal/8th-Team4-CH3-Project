// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/LA_BossProjectile.h"

#include "GameplayTagAssetInterface.h"
#include "Character/LA_BaseCharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ALA_BossProjectile::ALA_BossProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALA_BossProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TagInterface)
    {
        FGameplayTagContainer TargetTags;
        TagInterface->GetOwnedGameplayTags(TargetTags);

        // 오직 플레이어 진영(Team.Ally)만 타격
        if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))))
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

            //  보스 탄환에 맞으면 오염도 상승!
            ALA_BaseCharacter* Player = Cast<ALA_BaseCharacter>(OtherActor);
            if (Player)
            {
                Player->IncreaseContamination(1.2f);
            }

            Destroy();
        }
    }
}

