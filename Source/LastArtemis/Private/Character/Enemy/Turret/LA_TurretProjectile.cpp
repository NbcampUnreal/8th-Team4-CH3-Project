// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Turret/LA_TurretProjectile.h"

#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"


class IGameplayTagAssetInterface;
// Sets default values
ALA_TurretProjectile::ALA_TurretProjectile()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void ALA_TurretProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    // 쏜 놈(포탑)과 맞은 놈의 태그 비교
    IGameplayTagAssetInterface* InstigatorTags = Cast<IGameplayTagAssetInterface>(GetInstigator());
    IGameplayTagAssetInterface* TargetTags = Cast<IGameplayTagAssetInterface>(OtherActor);

    if (InstigatorTags && TargetTags)
    {
        FGameplayTagContainer MyTeam;
        InstigatorTags->GetOwnedGameplayTags(MyTeam);

        FGameplayTagContainer TargetTeam;
        TargetTags->GetOwnedGameplayTags(TargetTeam);

        // 우리 팀이 아닌 녀석만 타격 (해킹 여부에 따라 타겟이 자동으로 바뀜)
        if (!MyTeam.HasAny(TargetTeam))
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
            Destroy();
        }
    }
}


