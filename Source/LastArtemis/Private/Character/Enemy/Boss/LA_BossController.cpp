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
        LoseSightRadius = 2500.0f; // 시야 상실 거리를 살짝 넓혀 보스의 집착도를 높입니다.
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

    // 기본 간격 초기화 백업
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
        // [시야 상실 우회] 보스룸 안에서는 플레이어가 한번 감지되면 숨더라도 끝까지 추적하도록
        // else{ TargetPlayer = nullptr; } 를 과감히 지워 타겟을 유지시킵니다.
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
    // [안전 가드] 타겟이 없거나, 이미 공격 중이거나, 보스가 죽었다면 즉시 탈락
    if (!TargetPlayer || bIsAttacking) return;

    ALA_BossCharacter* BossCharacter = Cast<ALA_BossCharacter>(GetPawn());
    if (!BossCharacter || BossCharacter->bIsDead) return; //  사망 플래그 체크

    // 공격 플래그를 여기서 확실하게 잠금 처리하여 1개 패턴만 실행되도록 보호합니다.
    bIsAttacking = true;

    BossCharacter->PlayAttackMontage();

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
            // [복합 패턴 버그 전면 수정]
            // 두 함수가 연속 호출되며 타이머 핸들을 덮어쓰던 치명적 문제를 방지하기 위해
            // 우선 부채꼴을 먼저 쏘고, 폭탄은 0.5초 뒤 시차를 두고 떨어지도록 설계합니다.
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

        // 3연사 종료 후 1.5초 쿨타임 지정
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
    SpawnParams.Instigator = BossPawn; // [핵심 수정] nullptr 방지! 공격 주체를 보스 자신으로 명확히 지정!

    FVector ForwardVector = BossPawn->GetActorForwardVector();

    // 보스 키 높이 감안하여 스폰 좌표 보정 (+80.0f)
    FVector SpawnLocation = BossPawn->GetActorLocation() + (ForwardVector * 150.0f) + FVector(0.f, 0.f, 80.f);
    FRotator SpawnRotation = ForwardVector.Rotation();

    GetWorld()->SpawnActor<AActor>( // 부모 타입인 ALA_Projectile 또는 AActor로 안전하게 캐스팅 스폰
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
        SpawnParams.Instigator = BossPawn; // nullptr 방지!

        FVector SpawnLocation = BossPawn->GetActorLocation() + (FinalRotation.Vector() * 150.0f) + FVector(0.f, 0.f, 80.f);

        GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, FinalRotation, SpawnParams);
    }

    // 부채꼴 발사 후 2초 뒤에 다음 공격 가능 (만약 페이즈 3 복합 패턴 작동 중이면 폭탄 타이머가 최종 쿨타임을 다시 갱신해 줄 겁니다)
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
    SpawnParams.Instigator = BossPawn; // nullptr 방지!

    FVector StartLocation = BossPawn->GetActorLocation() + (BossPawn->GetActorForwardVector() * 150.0f) + FVector(0.f, 0.f, 120.f); // 머리 위에서 던지도록 높이 보정
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

    // 폭탄 던진 후 2.5초 쿨타임 고정
    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        [this]() { bIsAttacking = false; },
        2.5f,
        false
    );
}
