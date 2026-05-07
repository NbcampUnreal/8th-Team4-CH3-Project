// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/LA_BossController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/Boss/LA_BossProjectile.h"
#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

ALA_BossController::ALA_BossController()
{
    PrimaryActorTick.bCanEverTick = true;

    BossPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("BossPerceptionComponent"));
    SetPerceptionComponent(*BossPerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightRadius = 2000.0f;
        LoseSightRadius = 2200.0f;
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
    if (Stimulus.WasSuccessfullySensed())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (Actor == PlayerPawn)
        {
            TargetPlayer = Actor;
        }
    }
    else
    {
        TargetPlayer = nullptr;
    }
}

void ALA_BossController::RotateToTarget()
{
    if (TargetPlayer && GetPawn())
    {
        FVector Direction = TargetPlayer->GetActorLocation() - GetPawn()->GetActorLocation();
        Direction.Z = 0.0f; // Z축 회전 고정

        FRotator TargetRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(TargetRotation);
    }
}

void ALA_BossController::ExecutePhasePattern()
{
    if (!TargetPlayer || bIsAttacking) return;

    ALA_BossCharacter* BossCharacter = Cast<ALA_BossCharacter>(GetPawn());
    if (BossCharacter)
    {
        // 보스의 현재 페이즈 확인
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
                ShootBomb();
            }
            break;

        default:
            ShootBasic3Shot();
            break;
        }
    }
}

void ALA_BossController::ShootBasic3Shot()
{
    ALA_BossCharacter* Boss = Cast<ALA_BossCharacter>(GetPawn());
    if (Boss)
    {
        // 보스가 손을 뻗는 모션 재생
        Boss->PlayAttackMontage();

        // 이후 탄환 스폰 로직 실행
        if (bIsAttacking) return;
        bIsAttacking = true;

        CurrentShotsFired = 0;

        GetWorld()->GetTimerManager().SetTimer(
            MultiShotTimerHandle,
            this,
            &ALA_BossController::ShootSingleShot,
            BasicShotInterval,
            true // 반복
        );
    }

}

void ALA_BossController::ShootSingleShot()
{
    if (CurrentShotsFired >= MaxBasicShots || !GetPawn() || !ProjectileClass)
    {
        GetWorld()->GetTimerManager().ClearTimer(MultiShotTimerHandle);

        // 공격 쿨타임 지정
        GetWorld()->GetTimerManager().SetTimer(
            AttackTimerHandle,
            [this]() { bIsAttacking = false; },
            1.5f,
            false
        );
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetPawn();
    SpawnParams.Instigator = GetPawn()->GetInstigator();

    FVector ForwardVector = GetPawn()->GetActorForwardVector();
    FVector SpawnLocation = GetPawn()->GetActorLocation() + (ForwardVector * 100.0f);
    FRotator SpawnRotation = ForwardVector.Rotation();

    GetWorld()->SpawnActor<ALA_Projectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    CurrentShotsFired++;
}

void ALA_BossController::ShootFanPattern()
{
    if (bIsAttacking) return;
    bIsAttacking = true;

    if (GetPawn() && ProjectileClass)
    {
        int32 NumBullets = 12;
        float SpreadAngle = 60.0f;
        float AngleStep = SpreadAngle / (NumBullets - 1);

        FVector ForwardVector = GetPawn()->GetActorForwardVector();
        FRotator SpawnRotation = ForwardVector.Rotation();

        for (int32 i = 0; i < NumBullets; ++i)
        {
            float CurrentAngle = -(SpreadAngle / 2.0f) + (i * AngleStep);
            FRotator RotationOffset = FRotator(0.0f, CurrentAngle, 0.0f);
            FRotator FinalRotation = SpawnRotation + RotationOffset;

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetPawn();
            SpawnParams.Instigator = GetPawn()->GetInstigator();

            FVector SpawnLocation = GetPawn()->GetActorLocation() + (FinalRotation.Vector() * 100.0f);

            GetWorld()->SpawnActor<ALA_Projectile>(
                ProjectileClass,
                SpawnLocation,
                FinalRotation,
                SpawnParams
            );
        }
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
    if (!GetPawn() || !TargetPlayer) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetPawn();

    FVector StartLocation = GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * 100.0f;
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    // 폭탄 스폰
    ALA_Projectile* Bomb = GetWorld()->SpawnActor<ALA_Projectile>(BombClass, StartLocation, FRotator::ZeroRotator, SpawnParams);

    if (Bomb)
    {
        //  팁: 타겟 방향으로 포물선을 그리며 던지기 위해 속도 벡터 계산
        FVector LaunchVelocity;
        UGameplayStatics::SuggestProjectileVelocity_CustomArc(
            this,
            LaunchVelocity,
            StartLocation,
            TargetLocation,
            GetWorld()->GetGravityZ(),
            0.5f // 호의 높이 (0.5면 적당한 포물선)
        );

        Cast<UProjectileMovementComponent>(Bomb->GetComponentByClass(UProjectileMovementComponent::StaticClass()))
            ->Velocity = LaunchVelocity;
    }
}
