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
    // 유효성 및 자폭 방지
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

    //  방어 코드: 인터페이스 검사와 액터 자체 태그 검사를 동시에 진행합니다.
    bool bIsAlly = false;

    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TagInterface)
    {
        FGameplayTagContainer TargetTags;
        TagInterface->GetOwnedGameplayTags(TargetTags);
        if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))))
        {
            bIsAlly = true;
        }
    }
    // 인터페이스 캐스팅에 실패하더라도 액터가 직접 'Team.Ally' 태그를 가졌다면 인정해줍니다.
    else if (OtherActor->ActorHasTag(FName("Team.Ally")))
    {
        bIsAlly = true;
    }

    //  피아식별 통과 시 작동
    if (bIsAlly)
    {
        // 데미지 적용 (Damage 변수는 부모에게서 상속받아 사용되므로 정상 작동합니다.)
        UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

        // 보스 탄환에 맞으면 오염도 상승!
        ALA_BaseCharacter* Player = Cast<ALA_BaseCharacter>(OtherActor);
        if (Player)
        {
            Player->IncreaseContamination(1.2f);
            UE_LOG(LogTemp, Warning, TEXT("[보스 투사체] 플레이어 명중! 오염도 상승 누적"));
        }

        // 맞췄으니 탄환 삭제
        Destroy();
    }
}
