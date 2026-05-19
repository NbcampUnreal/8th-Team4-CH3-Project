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
void ALA_BossProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 유효성 및 자폭 방지 (나 자신이나 보스 본인은 맞추지 않음)
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

    bool bIsAlly = false;
    FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

    // [1단계] 정석 인터페이스 캐스팅 검사
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TagInterface)
    {
        FGameplayTagContainer TargetTags;
        TagInterface->GetOwnedGameplayTags(TargetTags);
        if (TargetTags.HasTag(AllyTag))
        {
            bIsAlly = true;
        }
    }

    // [2단계 치트키] 인터페이스 실패 시 플레이어의 'CharacterTags' 변수를 리플렉션으로 강제 털어오기
    if (!bIsAlly)
    {
        if (FProperty* TagProp = OtherActor->GetClass()->FindPropertyByName(FName("CharacterTags")))
        {
            if (FGameplayTagContainer* PropValue = TagProp->ContainerPtrToValuePtr<FGameplayTagContainer>(OtherActor))
            {
                if (PropValue->HasTag(AllyTag))
                {
                    bIsAlly = true;
                }
            }
        }
    }

    // [3단계 강제 합격] 변수도 씹히는 최악의 상황 대비: 이름 검사 및 컨트롤러 조종 여부로 강제 인정
    if (!bIsAlly)
    {
        if (OtherActor->ActorHasTag(FName("Team.Ally")) ||
            OtherActor->GetName().ToLower().Contains(TEXT("player")) ||
            OtherActor->GetName().ToLower().Contains(TEXT("character")) ||
            OtherActor->GetName().ToLower().Contains(TEXT("ally")) ||
            (Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsPlayerControlled()))
        {
            bIsAlly = true;
        }
    }

    // 피아식별 통과 시 작동 (드디어 100% 들어옵니다!)
    if (bIsAlly)
    {
        // 부모가 물려준 Damage 변수 활용 (만약 0 이하라면 보스답게 20.0f 정도로 보정)
        float FinalDamage = (Damage > 0.0f) ? Damage : 20.0f;

        UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, GetInstigatorController(), this, UDamageType::StaticClass());

        // 보스 탄환 고유 기믹: 맞으면 오염도 상승!
        // 부모인 BaseCharacter로 안전하게 캐스팅합니다.
        ALA_BaseCharacter* Player = Cast<ALA_BaseCharacter>(OtherActor);
        if (Player)
        {
            Player->IncreaseContamination(1.2f);
        }

        // 맞췄으니 탄환 삭제
        Destroy();
        return;
    }

    // 예외 처리: 캐릭터가 아니라 보스룸 벽이나 기둥(WorldStatic)에 부딪힌 거라면 소멸시킵니다.
    if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        Destroy();
    }
}
