#include "Character/Enemy/Turret/LA_TurretEnemy.h"
#include "AIController.h"
#include "Character/Enemy/Turret/LA_TurretProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALA_TurretEnemy::ALA_TurretEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    TurretHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretHead"));
    TurretHead->SetupAttachment(RootComponent);

    DetectionRange = 2000.0f;
    RotationSpeed = 5.0f;
    FireRate = 1.5f;
}

// Called when the game starts or when spawned
void ALA_TurretEnemy::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ALA_TurretEnemy::FireProjectile, FireRate, true);
}

// Called every frame
void ALA_TurretEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FindTarget();

    if (CurrentTarget && TurretHead)
    {
        // 타겟을 향한 회전값 계산
        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(TurretHead->GetComponentLocation(), CurrentTarget->GetActorLocation());
        FRotator SmoothedRot = FMath::RInterpTo(TurretHead->GetComponentRotation(), TargetRot, DeltaTime, RotationSpeed);

        // 포탑 헤드만 타겟을 바라보게 회전
        TurretHead->SetWorldRotation(SmoothedRot);
    }
}

void ALA_TurretEnemy::FindTarget()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && GetDistanceTo(PlayerPawn) <= DetectionRange)
    {
        CurrentTarget = PlayerPawn;
    }
    else
    {
        CurrentTarget = nullptr;
    }
}

void ALA_TurretEnemy::FireProjectile()
{
    if (CurrentTarget && ProjectileClass)
    {
        PlayAttackMontage();

        FVector SpawnLocation = TurretHead->GetSocketLocation(TEXT("Muzzle"));
        FRotator SpawnRotation = TurretHead->GetSocketRotation(TEXT("Muzzle"));

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;

        GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    }
}

void ALA_TurretEnemy::SwitchTeam(FGameplayTag NewTeamTag)
{
    UpdateTeamTag(NewTeamTag);

    // 2. 현재 조준 중인 타겟 초기화
    CurrentTarget = nullptr;

    // 3. AI가 이전에 공격하던 적(플레이어)에 대한 정보를 지우도록 함
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement(); // 조준 초기화 역할
    }

}


