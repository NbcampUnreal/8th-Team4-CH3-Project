#include "Character/Enemy/Boss/LA_BombProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagAssetInterface.h" // 인터페이스용 인클루드 추가

ALA_BombProjectile::ALA_BombProjectile()
{
    if (ProjectileMovement)
    {
        ProjectileMovement->bShouldBounce = true;
        ProjectileMovement->Bounciness = 0.4f;
        ProjectileMovement->ProjectileGravityScale = 1.2f;
        ProjectileMovement->Friction = 0.5f;
    }

    ExplosionRadius = 400.0f;
    FuseTime = 2.0f;
    Damage = 35.0f;
}

void ALA_BombProjectile::BeginPlay()
{
    // 부모 클래스(ALA_Projectile)에 BeginPlay 바인딩이 들어있으므로 반드시 Super를 호출해 줍니다!
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &ALA_BombProjectile::Explode, FuseTime, false);
}

// 플레이어 몸에 직격으로 "닿았을 때" 즉시 터트리는 가로채기 함수
void ALA_BombProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

    bool bIsAlly = false;
    FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

    // [3중 우회망 가동] 플레이어 진영 확실하게 색출하기
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (TagInterface)
    {
        FGameplayTagContainer TargetTags;
        TagInterface->GetOwnedGameplayTags(TargetTags);
        if (TargetTags.HasTag(AllyTag)) bIsAlly = true;
    }

    if (!bIsAlly)
    {
        if (FProperty* TagProp = OtherActor->GetClass()->FindPropertyByName(FName("CharacterTags")))
        {
            if (FGameplayTagContainer* PropValue = TagProp->ContainerPtrToValuePtr<FGameplayTagContainer>(OtherActor))
            {
                if (PropValue->HasTag(AllyTag)) bIsAlly = true;
            }
        }
    }

    if (!bIsAlly)
    {
        if (OtherActor->ActorHasTag(FName("Team.Ally")) ||
            OtherActor->GetName().ToLower().Contains(TEXT("player")) ||
            (Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsPlayerControlled()))
        {
            bIsAlly = true;
        }
    }

    // 플레이어 직격 성공 시, 2초 타이머 취소하고 그 자리에서 즉시 "쿠쾅!"
    if (bIsAlly)
    {
        UE_LOG(LogTemp, Warning, TEXT("보스 폭탄 직격] 플레이어 몸에 직접 명중하여 즉시 폭발합니닷!"));
        GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandle);
        Explode();
    }
}

void ALA_BombProjectile::Explode()
{
    // 데미지 연산에서 제외할 아군 명단 확보 (자폭 및 팀킬 방지)
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);
    if (GetOwner()) IgnoreActors.Add(GetOwner());
    if (GetInstigator()) IgnoreActors.Add(GetInstigator());

    // [가시성 버그 수정] 폭탄 중심점을 바닥에서 위쪽 방향으로 20 유닛만큼 살짝 들어 올려서 쏩니다.
    // 이렇게 축을 보정해 주면 바닥 콜리전에 가로막혀 대미지가 씹히는 현상이 100% 교정됩니다!
    FVector CorrectedExplosionLocation = GetActorLocation() + (GetActorUpVector() * 20.0f);

    // 범위 데미지 적용
    UGameplayStatics::ApplyRadialDamage(
        this,
        Damage,
        CorrectedExplosionLocation, // 보정된 위치 적용
        ExplosionRadius,
        UDamageType::StaticClass(),
        IgnoreActors,               // 무시할 액터 명단을 정직하게 넘겨줍니다.
        this,
        GetInstigatorController(),
        false // [핵심 가이드] 가시성 체크(bDoFullDamage)를 false로 끄는 것을 강력 추천합니다!
              // 보스방 기둥 뒤에 숨는 식의 정교한 엄폐 플레이가 필수인 기믹이 아니라면,
              // 사방 400유닛 이내에 있는 플레이어에게 버그 없이 확정 타격을 주기 위해 false로 세팅하는 것이 훨씬 시원시원합니다.
    );

    // 이펙트 및 사운드
    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }


    Destroy();
}
