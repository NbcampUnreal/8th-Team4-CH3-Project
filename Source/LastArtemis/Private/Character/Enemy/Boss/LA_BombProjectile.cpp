#include "Character/Enemy/Boss/LA_BombProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagAssetInterface.h"

ALA_BombProjectile::ALA_BombProjectile()
{
    // 🎯 1. 중요: 무조건 매 프레임 굴러가는 회전 연산을 해야 하므로 틱을 켭니다!
    PrimaryActorTick.bCanEverTick = true;
    UPrimitiveComponent* BombCollision = Cast<UPrimitiveComponent>(RootComponent);

    if (BombCollision)
    {
        // 🛠️ 프리셋을 'PhysicsActor'나 'BlockAllDynamic'으로 변경하여
        // 맵 바닥(Static Mesh, Landscape)에 부딪혔을 때 절대 뚫리지 않고 팅겨 나가거나 구르게 만듭니다!
        BombCollision->SetCollisionProfileName(TEXT("PhysicsActor"));
    }

    if (ProjectileMovement)
    {
        ProjectileMovement->bShouldBounce = true;

        // 🎯 2. 구르는 느낌을 주기 위한 최적의 수치 튜닝!
        ProjectileMovement->Bounciness = 0.15f;       // 반발력을 확 낮춰서 땅에 닿자마자 튀지 않고 가라앉게 만듭니다.
        ProjectileMovement->Friction = 0.2f;          // 마찰력을 적당히 줄여서 얼음판 위를 미끄러지듯 부드럽게 구르게 합니다.
        ProjectileMovement->ProjectileGravityScale = 1.5f; // 중력을 높여 바닥에 자석처럼 딱 붙어 구르게 유도합니다.

    }

    ExplosionRadius = 400.0f;
    FuseTime = 2.0f;
    Damage = 35.0f;
    CurrentRollRotation = 0.0f;
}

void ALA_BombProjectile::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &ALA_BombProjectile::Explode, FuseTime, false);
}

// 🎯 3. 매 프레임 실제 이동 속도에 맞춰 폭탄 메쉬를 굴려주는 로직 추가!
void ALA_BombProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ProjectileMovement)
    {
        // 현재 폭탄이 수평(X, Y)으로 얼마나 빨리 움직이고 있는지 속도(크기)를 구합니다.
        float CurrentSpeed = ProjectileMovement->Velocity.Size2D();

        // 속도가 아주 미미할 때는 회전하지 않습니다.
        if (CurrentSpeed > 10.0f)
        {
            // 이동 속도에 비례해서 회전 각도를 누적합니다. (수치 0.05f는 구르는 속도 취향껏 조절 가능!)
            CurrentRollRotation -= CurrentSpeed * DeltaTime * 0.05f;

            // 폭탄 내부의 루트 컴포넌트나 메쉬의 상대 회전값을 갱신하여 데굴데굴 구르는 연출을 만듭니다.
            // (만약 루트 컴포넌트가 스피어 콜리전이라면, 내부의 Mesh 컴포넌트만 회전시키는 게 정석입니다.)
            if (RootComponent)
            {
                // Pitch(Y축 회전)를 주어 전방으로 구르게 만듭니다.
                RootComponent->SetRelativeRotation(FRotator(CurrentRollRotation, 0.f, 0.f));
            }
        }
    }
}

// 플레이어 직격 Overlap 및 Explode 함수는 기존 코드 완전 동일하게 유지!
void ALA_BombProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

    bool bIsAlly = false;
    FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

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

    if (bIsAlly)
    {
        UE_LOG(LogTemp, Warning, TEXT("[보스 폭탄 직격] 플레이어 몸에 직접 명중하여 즉시 폭발합니닷!"));
        GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandle);
        Explode();
    }
}

void ALA_BombProjectile::Explode()
{
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);
    if (GetOwner()) IgnoreActors.Add(GetOwner());
    if (GetInstigator()) IgnoreActors.Add(GetInstigator());

    FVector CorrectedExplosionLocation = GetActorLocation() + (GetActorUpVector() * 20.0f);

    UGameplayStatics::ApplyRadialDamage(
        this,
        Damage,
        CorrectedExplosionLocation,
        ExplosionRadius,
        UDamageType::StaticClass(),
        IgnoreActors,
        this,
        GetInstigatorController(),
        false
    );

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
