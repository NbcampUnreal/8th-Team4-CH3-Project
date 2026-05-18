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

    CurrentTarget = nullptr;
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
        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(TurretHead->GetComponentLocation(), CurrentTarget->GetActorLocation());

        // 포탑 특성상 Pitch(상하)와 Yaw(좌우)만 회전하고 Roll(정크 회전)은 막는 것이 자연스럽습니다.
        TargetRot.Roll = 0.0f;

        FRotator SmoothedRot = FMath::RInterpTo(TurretHead->GetComponentRotation(), TargetRot, DeltaTime, RotationSpeed);
        TurretHead->SetWorldRotation(SmoothedRot);
    }
}

void ALA_TurretEnemy::FindTarget()
{
    // 팀 기반 동적 타겟팅 구현
    // 터렛 본인의 태그를 검사합니다. (부모 클래스나 본인에게 구현된 CharacterTags 활용)
    bool bIsOriginallyEnemy = CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));

    if (bIsOriginallyEnemy)
    {
        // 적군 상태일 때: 플레이어(Team.Ally)를 타겟으로 잡음
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn && GetDistanceTo(PlayerPawn) <= DetectionRange && CheckLineOfSight(PlayerPawn))
        {
            CurrentTarget = PlayerPawn;
            return;
        }
    }
    else
    {
        // 아군(해킹됨) 상태일 때: 주변의 몬스터(Team.Enemy)들을 탐색함
        TArray<AActor*> OverlappingActors;
        // 터렛 위치 기준 DetectionRange 반경 내의 모든 에너미 캐릭터를 스캔하는 로직을 돌리거나,
        // 가장 간단하게는 가까운 에너미 액터를 타겟팅하도록 구현합니다.

        // (이해를 돕기 위한 예시: 타겟 수색 자동화)
        TArray<AActor*> FoundEnemies;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Team.Enemy"), FoundEnemies);

        AActor* ClosestEnemy = nullptr;
        float ClosestDistance = DetectionRange;

        for (AActor* Enemy : FoundEnemies)
        {
            float Dist = GetDistanceTo(Enemy);
            if (Dist <= ClosestDistance && CheckLineOfSight(Enemy))
            {
                ClosestDistance = Dist;
                ClosestEnemy = Enemy;
            }
        }

        if (ClosestEnemy)
        {
            CurrentTarget = ClosestEnemy;
            return;
        }
    }

    // 조건에 만족하는 타겟이 없으면 널 처리
    CurrentTarget = nullptr;
}

void ALA_TurretEnemy::FireProjectile()
{
    if (CurrentTarget && ProjectileClass && TurretHead)
    {
        // 조준 각도 안전장치 추가
        // 현재 포탑 정면 벡터와 타겟을 향한 벡터 사이의 각도를 구합니다.
        FVector Forward = TurretHead->GetForwardVector();
        FVector ToTarget = (CurrentTarget->GetActorLocation() - TurretHead->GetComponentLocation()).GetSafeNormal();

        float AngleDot = FVector::DotProduct(Forward, ToTarget); // 내적 계산

        // 두 벡터의 사잇각이 약 15도 이내일 때만 총알을 발사합니다. (cos(15도) ≒ 0.96)
        if (AngleDot < 0.96f)
        {
            return; // 아직 고개가 다 안 돌아갔으면 사격 스킵!
        }

        PlayAttackMontage();

        FVector SpawnLocation = TurretHead->GetSocketLocation(TEXT("Muzzle"));
        FRotator SpawnRotation = TurretHead->GetSocketRotation(TEXT("Muzzle"));

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;

        GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    }
}

bool ALA_TurretEnemy::CheckLineOfSight(AActor* TargetActor)
{
    if (!TargetActor || !GetWorld()) return false;

    FHitResult HitResult;
    FVector StartLocation = TurretHead->GetComponentLocation();
    FVector EndLocation = TargetActor->GetActorLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 나 자신은 무시
    QueryParams.AddIgnoredActor(TargetActor); // 조준 대상도 통과 (그 사이의 벽만 체크하기 위함)

    // Visibility 채널로 레이를 쏴서 도중에 막히는 벽(Static)이 있는지 검사
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    // 가로막은 벽이 없다면 true 반환
    return !bHit;
}

void ALA_TurretEnemy::SwitchTeam(FGameplayTag NewTeamTag)
{
    //  부모의 팀 변경 기능 호출
    UpdateTeamTag(NewTeamTag);

    //  현재 조준 중인 타겟 즉시 초기화 (다음 FindTarget 때 새 진영 기준으로 찾음)
    CurrentTarget = nullptr;

    //  AI 컨트롤러 제어
    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
    }

}

