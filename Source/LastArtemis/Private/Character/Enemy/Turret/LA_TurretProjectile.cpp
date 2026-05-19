// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Turret/LA_TurretProjectile.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"


ALA_TurretProjectile::ALA_TurretProjectile()
{
    // 생성자에서는 아무것도 만들 필요 없습니다. 부모가 이미 다 만들어 놨습니다!
    // 원한다면 부모 변수인 Damage의 기본값만 터렛에 맞게 튜닝합니다.
    Damage = 25.0f;
}

void ALA_TurretProjectile::BeginPlay()
{
    //  [초특급 중요] 이걸 호출해야 부모의 BeginPlay 안에 있는
    // CollisionComp->OnComponentBeginOverlap.AddDynamic(...) 코드가 정상 가동됩니다!
    Super::BeginPlay();
}

// 🎯 부모의 함수를 완벽하게 가로챈 자식만의 대미지 정산 함수
void ALA_TurretProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 기초 예외 처리 (나 자신이나 나를 쏜 포탑 무시)
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

    FGameplayTagContainer MyTeamTags;
    FGameplayTagContainer TargetTeamTags;
    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

    // 발사대(터렛) 팀 추출
    AActor* MyLauncher = GetInstigator();
    if (!MyLauncher) MyLauncher = GetOwner();

    if (MyLauncher)
    {
        IGameplayTagAssetInterface* InstigatorTags = Cast<IGameplayTagAssetInterface>(MyLauncher);
        if (InstigatorTags) InstigatorTags->GetOwnedGameplayTags(MyTeamTags);

        if (MyTeamTags.IsEmpty())
        {
            if (FProperty* TagProp = MyLauncher->GetClass()->FindPropertyByName(FName("CharacterTags")))
            {
                if (FGameplayTagContainer* PropValue = TagProp->ContainerPtrToValuePtr<FGameplayTagContainer>(MyLauncher))
                {
                    MyTeamTags = *PropValue;
                }
            }
        }
    }

    // 맞은 대상(플레이어 등) 팀 추출
    IGameplayTagAssetInterface* TargetTags = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TargetTags) TargetTags->GetOwnedGameplayTags(TargetTeamTags);

    if (TargetTeamTags.IsEmpty())
    {
        if (FProperty* TagProp = OtherActor->GetClass()->FindPropertyByName(FName("CharacterTags")))
        {
            if (FGameplayTagContainer* PropValue = TagProp->ContainerPtrToValuePtr<FGameplayTagContainer>(OtherActor))
            {
                TargetTeamTags = *PropValue;
            }
        }
    }

    // 플레이어 인터페이스 누락 예외 우회 치트키
    if (TargetTeamTags.IsEmpty())
    {
        if (OtherActor->ActorHasTag(FName("Team.Ally")) || OtherActor->GetName().ToLower().Contains(TEXT("player")) || (Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsPlayerControlled()))
        {
            TargetTeamTags.AddTag(AllyTag);
        }
    }

    // 다른 팀일 때만 대미지 폭격 수행 (피아식별 교차 검증)
    if (!MyTeamTags.IsEmpty() && !TargetTeamTags.IsEmpty())
    {
        if (!MyTeamTags.HasAny(TargetTeamTags))
        {
            // 부모가 물려준 Damage 변수의 값을 그대로 사용합니다.
            UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

            UE_LOG(LogTemp, Warning, TEXT(" [터렛 자식 투사체] %s 타격 대미지 주입 완료! 대미지: %f"), *OtherActor->GetName(), Damage);

            Destroy();
            return;
        }
        else
        {
            // 같은 팀이면 총알만 삭제 (팀킬 방지)
            Destroy();
            return;
        }
    }

    // 캐릭터가 아닌 벽(WorldStatic)에 부딪힌 경우 소멸 (부모의 원래 기능 수행)
    if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        Destroy();
    }
}

