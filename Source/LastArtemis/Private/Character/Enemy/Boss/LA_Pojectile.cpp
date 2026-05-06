// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/LA_Pojectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALA_Projectile::ALA_Projectile()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 루트 지정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // 프로젝트 설정에 맞게 변경
    RootComponent = CollisionComp;

    // 메시 컴포넌트 생성 및 부착
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);

    // 투사체 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 2000.0f;
    ProjectileMovement->MaxSpeed = 2000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    // 데미지 기본값
    Damage = 20.0f;

    // 일정 시간(3초) 뒤 자동 소멸 설정
    InitialLifeSpan = 3.0f;
}

// Called when the game starts or when spawned
void ALA_Projectile::BeginPlay()
{
    Super::BeginPlay();

    // 겹침 이벤트 바인딩
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ALA_Projectile::OnOverlapBegin);
}

void ALA_Projectile::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // 자기 자신이나 소유자(Owner)와 부딪힌 것이 아닐 때만 로직 실행
    if (OtherActor && OtherActor != this && OtherActor != GetOwner())
    {
        // TODO: 타겟에게 데미지를 부여합니다. (프로젝트 환경에 따라 캐스팅 방식을 수정해 주세요)
        /*
        ALA_BaseCharacter* TargetCharacter = Cast<ALA_BaseCharacter>(OtherActor);
        if (TargetCharacter)
        {
            TargetCharacter->TakeDamageCustom(Damage);
        }
        */

        // 충돌 후 투사체 소멸
        Destroy();
    }
}

// Called every frame
void ALA_Projectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
