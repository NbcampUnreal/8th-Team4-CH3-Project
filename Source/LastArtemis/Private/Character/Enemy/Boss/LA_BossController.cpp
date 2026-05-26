#include "Character/Enemy/Boss/LA_BossController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/Boss/LA_BossProjectile.h"
#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ALA_BossController::ALA_BossController()
{
    PrimaryActorTick.bCanEverTick = true;

    BossPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("BossPerceptionComponent"));
    SetPerceptionComponent(*BossPerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightRadius = 2000.0f;
        LoseSightRadius = 2500.0f;
        AttackRange = 1500.0f;

        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = 180.0f;

        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

        BossPerceptionComponent->ConfigureSense(*SightConfig);
        BossPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    TargetPlayer = nullptr;
    bIsAttacking = false;
    CurrentShotsFired = 0;

    BasicShotInterval = 0.2f;
    MaxBasicShots = 3;
}

void ALA_BossController::BeginPlay()
{
    Super::BeginPlay();

    if (BossPerceptionComponent)
    {
        BossPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALA_BossController::OnPerceptionUpdated);
    }
}

void ALA_BossController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ALA_BossController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RotateToTarget();
    ExecutePhasePattern();
}

void ALA_BossController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Actor == PlayerPawn)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            TargetPlayer = Actor;
        }
    }
}

void ALA_BossController::RotateToTarget()
{
    if (TargetPlayer && GetPawn())
    {
        FVector Direction = TargetPlayer->GetActorLocation() - GetPawn()->GetActorLocation();
        Direction.Z = 0.0f;

        FRotator TargetRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(TargetRotation);
    }
}

void ALA_BossController::ExecutePhasePattern()
{
    if (!TargetPlayer || bIsAttacking) return;

    ALA_BossCharacter* BossCharacter = Cast<ALA_BossCharacter>(GetPawn());
    if (!BossCharacter) return;

    // 🎯 1. [심볼 에러 영구 해결]
    // 부모 클래스가 물려준 HealthComponent 주소를 활용해 안전하게 사망 판정을 필터링합니다.
    if (BossCharacter->GetHealthComponent() && BossCharacter->GetHealthComponent()->IsDead()) return;

    bIsAttacking = true;

    // 💥 [스태틱 메시 최적화 수술]
    // 고정형 기믹 보스는 스켈레탈 메시가 없으므로 몽타주 재생 함수(PlayAttackMontage) 라인을 삭제했습니다!

    EBossPhase CurrentPhase = BossCharacter->GetCurrentPhase();
    float RandomValue = FMath::FRandRange(0.0f, 100.0f);

    switch (CurrentPhase)
    {
    case EBossPhase::Phase1:
        ShootBasic3Shot();
        break;

    case EBossPhase::Phase2:
        if (RandomValue <= 50.0f) ShootBasic3Shot();
        else ShootFanPattern();
        break;

    case EBossPhase::Phase3:
        if (RandomValue <= 40.0f)
        {
            ShootFanPattern();
        }
        else if (RandomValue <= 80.0f)
        {
            ShootBomb();
        }
        else
        {
            ShootFanPattern();

            FTimerHandle ChaosTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(ChaosTimerHandle, this, &ALA_BossController::ShootBomb, 0.5f, false);
        }
        break;

    default:
        ShootBasic3Shot();
        break;
    }
}

void ALA_BossController::ShootBasic3Shot()
{
    CurrentShotsFired = 0;

    GetWorld()->GetTimerManager().SetTimer(
        MultiShotTimerHandle,
        this,
        &ALA_BossController::ShootSingleShot,
        BasicShotInterval,
        true
    );
}

void ALA_BossController::ShootSingleShot()
{
    APawn* BossPawn = GetPawn();
    if (CurrentShotsFired >= MaxBasicShots || !BossPawn || !ProjectileClass)
    {
        GetWorld()->GetTimerManager().ClearTimer(MultiShotTimerHandle);

        GetWorld()->GetTimerManager().SetTimer(
            AttackTimerHandle,
            [this]() { bIsAttacking = false; },
            1.5f,
            false
        );
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = BossPawn;
    SpawnParams.Instigator = BossPawn;

    FVector ForwardVector = BossPawn->GetActorForwardVector();

    FVector SpawnLocation = BossPawn->GetActorLocation() + (ForwardVector * 150.0f) + FVector(0.f, 0.f, 80.f);
    FRotator SpawnRotation = ForwardVector.Rotation();

    GetWorld()->SpawnActor<AActor>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    CurrentShotsFired++;
}

void ALA_BossController::ShootFanPattern()
{
    APawn* BossPawn = GetPawn();
    if (!BossPawn || !ProjectileClass)
    {
        bIsAttacking = false;
        return;
    }

    int32 NumBullets = 12;
    float SpreadAngle = 60.0f;
    float AngleStep = SpreadAngle / (NumBullets - 1);

    FVector ForwardVector = BossPawn->GetActorForwardVector();
    FRotator SpawnRotation = ForwardVector.Rotation();

    for (int32 i = 0; i < NumBullets; ++i)
    {
        float CurrentAngle = -(SpreadAngle / 2.0f) + (i * AngleStep);
        FRotator RotationOffset = FRotator(0.0f, CurrentAngle, 0.0f);
        FRotator FinalRotation = SpawnRotation + RotationOffset;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = BossPawn;
        SpawnParams.Instigator = BossPawn;

        FVector SpawnLocation = BossPawn->GetActorLocation() + (FinalRotation.Vector() * 150.0f) + FVector(0.f, 0.f, 80.f);

        GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, FinalRotation, SpawnParams);
    }

    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        [this]() { bIsAttacking = false; },
        2.0f,
        false
    );
}

void ALA_BossController::ShootBomb()
{
    APawn* BossPawn = GetPawn();
    if (!BossPawn || !TargetPlayer || !BombClass)
    {
        bIsAttacking = false;
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = BossPawn;
    SpawnParams.Instigator = BossPawn;

    FVector StartLocation = BossPawn->GetActorLocation() + (BossPawn->GetActorForwardVector() * 150.0f) + FVector(0.f, 0.f, 120.f);
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    AActor* SpawnedBomb = GetWorld()->SpawnActor<AActor>(BombClass, StartLocation, FRotator::ZeroRotator, SpawnParams);

    if (SpawnedBomb)
    {
        FVector LaunchVelocity;
        bool bHaveData = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
            this, LaunchVelocity, StartLocation, TargetLocation, GetWorld()->GetGravityZ(), 0.5f
        );

        if (bHaveData)
        {
            UProjectileMovementComponent* ProjMove = Cast<UProjectileMovementComponent>(SpawnedBomb->GetComponentByClass(UProjectileMovementComponent::StaticClass()));
            if (ProjMove)
            {
                ProjMove->Velocity = LaunchVelocity;
            }
        }
    }

    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        [this]() { bIsAttacking = false; },
        2.5f,
        false
    );
}
