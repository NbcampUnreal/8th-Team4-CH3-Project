// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Turret/LA_TurretProjectile.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"

ALA_TurretProjectile::ALA_TurretProjectile()
{
    // 생성자에서는 아무것도 만들 필요 없습니다. 부모가 이미 다 만들어 놨습니다!
    // 터렛 밸런스에 맞게 대미지만 기본값 25.0f로 세팅합니다.
    Damage = 25.0f;
}

void ALA_TurretProjectile::BeginPlay()
{
    // 🎯 [필수 호출] 부모의 BeginPlay가 가동되면서
    // 부모에 장착된 OnComponentBeginOverlap 및 OnComponentHit 델리게이트가 전부 자동 바인딩됩니다!
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

    // 1. 발사대(터렛) 팀 추출
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

    // 2. 맞은 대상(플레이어 등) 팀 추출
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

    // 3. 플레이어 인터페이스 누락 예외 우회 치트키
    if (TargetTeamTags.IsEmpty())
    {
        if (OtherActor->ActorHasTag(FName("Team.Ally")) || OtherActor->GetName().ToLower().Contains(TEXT("player")) || (Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsPlayerControlled()))
        {
            TargetTeamTags.AddTag(AllyTag);
        }
    }

    // 4. 다른 팀일 때만 대미지 폭격 수행 (피아식별 교차 검증)
    if (!MyTeamTags.IsEmpty() && !TargetTeamTags.IsEmpty())
    {
        if (!MyTeamTags.HasAny(TargetTeamTags))
        {
            // 🎯 [정석 ApplyDamage 연결] 부모 파이프라인을 관통시켜
            // 타격 대상의 헬스 컴포넌트 실시간 감쇄 + 공통 피격음을 정확하게 스피커로 출력합니다!
            UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

            UE_LOG(LogTemp, Warning, TEXT("🎯 [터렛 투사체 명중] %s 타격 완료! 대미지: %f"), *OtherActor->GetName(), Damage);

            Destroy();
            return;
        }
        else
        {
            // 같은 팀이면 총알만 안전하게 삭제 (팀킬 방지 완벽 구동)
            Destroy();
            return;
        }
    }

    // 🎯 [버그 해결] 캐릭터 처리가 아닌 그 외의 예외 상황(기타 환경 오브젝트 등)은
    // 부모 클래스가 가지고 있는 기본 베이스 오버랩 예외 처리 함수에게 제어권을 토스합니다!
    Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}
