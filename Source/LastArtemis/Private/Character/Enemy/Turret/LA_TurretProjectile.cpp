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
    // 유효성 검사 및 자폭 방지 (나 자신이나 나를 생성한 포탑은 맞추지 않음)
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

    // 나를 쏜 포탑(Instigator)과 맞은 대상(OtherActor)의 팀 정보를 담을 컨테이너
    FGameplayTagContainer MyTeamTags;
    FGameplayTagContainer TargetTeamTags;

    bool bMyTeamFound = false;
    bool bTargetTeamFound = false;

    //  포탑(Instigator)의 팀 태그 추출
    IGameplayTagAssetInterface* InstigatorTags = Cast<IGameplayTagAssetInterface>(GetInstigator());
    if (InstigatorTags)
    {
        InstigatorTags->GetOwnedGameplayTags(MyTeamTags);
        bMyTeamFound = true;
    }
    else if (GetInstigator())
    {
        // 인터페이스 실패 시 일반 액터 태그 검사 (안전장치)
        if (GetInstigator()->ActorHasTag(FName("Team.Enemy"))) MyTeamTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));
        if (GetInstigator()->ActorHasTag(FName("Team.Ally"))) MyTeamTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")));
        bMyTeamFound = !MyTeamTags.IsEmpty();
    }

    //  맞은 대상(Target)의 팀 태그 추출
    IGameplayTagAssetInterface* TargetTags = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TargetTags)
    {
        TargetTags->GetOwnedGameplayTags(TargetTeamTags);
        bTargetTeamFound = true;
    }
    else
    {
        // 인터페이스 실패 시 일반 액터 태그 검사 (안전장치)
        if (OtherActor->ActorHasTag(FName("Team.Enemy"))) TargetTeamTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));
        if (OtherActor->ActorHasTag(FName("Team.Ally"))) TargetTeamTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")));
        bTargetTeamFound = !TargetTeamTags.IsEmpty();
    }

    //  둘 다 팀 정보가 확실히 확인되었을 때만 비교 연산 작동
    if (bMyTeamFound && bTargetTeamFound)
    {
        //  작성하신 핵심 로직 유지: 포탑과 타겟의 팀이 다를 때만 공격!
        if (!MyTeamTags.HasAny(TargetTeamTags))
        {
            UGameplayStatics::ApplyDamage(
                OtherActor,
                Damage, // 부모 클래스의 Damage 변수 활용
                GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );

            UE_LOG(LogTemp, Log, TEXT("[터렛 투사체] 적 진영 타격 성공! 대상: %s"), *OtherActor->GetName());

            // 명중했으므로 투사체 소멸
            Destroy();
        }
    }
    //  예외 처리: 만약 아군/적군 캐릭터가 아니라 일반 벽(Static)에 부딪힌 거라면 소멸시킵니다.
    else if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        Destroy();
    }
}


