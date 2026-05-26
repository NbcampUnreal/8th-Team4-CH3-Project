#include "Character/Enemy/Turret/LA_TurretEnemy.h"
#include "AIController.h"
#include "GameplayTagAssetInterface.h"
#include "Character/Enemy/Turret/LA_TurretProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/LA_GameModeBase.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/LA_EnemyHealthWidget.h"
#include "UI/LA_EnemyDamageTextWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ALA_TurretEnemy::ALA_TurretEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    TurretHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretHead"));
    TurretHead->SetupAttachment(RootComponent);

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    Defense = 5.0f;
    bIsDead = false;
    DetectionRange = 2000.0f;
    RotationSpeed = 5.0f;
    FireRate = 1.5f;

    CurrentTarget = nullptr;

    CharacterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));

    HealthComp = CreateDefaultSubobject<ULA_HealthComponent>(TEXT("HealthComp"));

    HealthWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
    HealthWidgetComp->SetupAttachment(RootComponent);
    HealthWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HealthWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    HealthWidgetComp->SetVisibility(false);

    static ConstructorHelpers::FClassFinder<UUserWidget> HB(TEXT("/Game/UI/Common/WBP_EnemyHealthBar.WBP_EnemyHealthBar_C"));
    if (HB.Succeeded()) HealthWidgetComp->SetWidgetClass(HB.Class);

    static ConstructorHelpers::FClassFinder<UUserWidget> DT(TEXT("/Game/UI/Common/WBP_EnemyDamageText.WBP_EnemyDamageText_C"));
    if (DT.Succeeded()) DamageTextClass = DT.Class;
}

// Called when the game starts or when spawned
void ALA_TurretEnemy::BeginPlay()
{
    Super::BeginPlay();

    // [핵심] 블루프린트 디테일 창이 비어있어도, 게임이 켜지자마자 C++ 단에서 강제로 적군 낙인을 찍어버립니다.
    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (!CharacterTags.HasTag(EnemyTag))
    {
        CharacterTags.AddTag(EnemyTag);
    }

    // 일반 액터 태그 배열에도 백업으로 심어줍니다.
    if (!ActorHasTag(FName("Team.Enemy")))
    {
        Tags.Add(FName("Team.Enemy"));
    }

    GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ALA_TurretEnemy::FireProjectile, FireRate, true);

    if (HealthComp)
    {
        HealthComp->OnHealthChanged.AddUObject(this, &ALA_TurretEnemy::OnHealthChangedCallback);
        if (HealthWidgetComp)
        {
            if (ULA_EnemyHealthWidget* HealthBar = Cast<ULA_EnemyHealthWidget>(HealthWidgetComp->GetUserWidgetObject()))
            {
                HealthComp->OnHealthChanged.AddUObject(HealthBar, &ULA_EnemyHealthWidget::UpdateHealthBar);
                HealthBar->UpdateHealthBar(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
            }
        }
    }
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
    if (!GetWorld()) return;

    // BeginPlay에서 강제 주입했으므로 이제 100% true가 나옵니다.
    bool bIsOriginallyEnemy = CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));

    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

    AActor* ClosestTarget = nullptr;
    float ClosestDistance = DetectionRange;

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

    float TurretHeightOffset = 80.0f;
    FVector StartLocation = GetActorLocation() + (GetActorUpVector() * TurretHeightOffset);

    for (AActor* Candidate : AllPawns)
    {
        if (!Candidate || Candidate == this) continue;

        FGameplayTagContainer TargetTags;
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Candidate);
        if (TagInterface)
        {
            TagInterface->GetOwnedGameplayTags(TargetTags);
        }
        else
        {
            if (FProperty* TagProp = Candidate->GetClass()->FindPropertyByName(FName("CharacterTags")))
            {
                if (FGameplayTagContainer* PropValue = TagProp->ContainerPtrToValuePtr<FGameplayTagContainer>(Candidate))
                {
                    TargetTags = *PropValue;
                }
            }
        }

        //  아군/적군 판정 필터링 (가장 안전한 3중 우회 필터)
        bool bIsTargetAlly = false;
        if (TargetTags.HasTag(AllyTag) || Candidate->ActorHasTag(FName("Team.Ally")))
        {
            bIsTargetAlly = true;
        }
        else if (Candidate->GetName().ToLower().Contains(TEXT("player")) ||
                 Candidate->GetName().ToLower().Contains(TEXT("character")) ||
                 Candidate->GetName().ToLower().Contains(TEXT("ally")))
        {
            bIsTargetAlly = true;
        }
        else if (Cast<APawn>(Candidate) && Cast<APawn>(Candidate)->IsPlayerControlled())
        {
            bIsTargetAlly = true;
        }

        bool bIsTargetEnemy = false;
        if (TargetTags.HasTag(EnemyTag) || Candidate->ActorHasTag(FName("Team.Enemy")) ||
            Candidate->GetName().ToLower().Contains(TEXT("enemy")) || Candidate->GetName().ToLower().Contains(TEXT("monster")))
        {
            bIsTargetEnemy = true;
        }

        //  진영 필터 통과 단계
        if (bIsOriginallyEnemy)
        {
            //  적군 터렛 상태일 때: 아군(플레이어 계열)이 아니면 전부 탈락시킵니다.
            if (!bIsTargetAlly)
            {
                continue;
            }
        }
        else
        {
            //  해킹된 아군 터렛 상태일 때: 적군(몬스터 계열)이 아니면 전부 탈락시킵니다.
            if (!bIsTargetEnemy)
            {
                continue;
            }
        }


        // 거리 및 시야 검사
        float Dist = GetDistanceTo(Candidate);
        if (Dist <= ClosestDistance)
        {
            if (CheckLineOfSight(Candidate))
            {
                ClosestDistance = Dist;
                ClosestTarget = Candidate;
            }
        }
    }

    // 최종 록온 성공 (보라색 선)
    if (ClosestTarget)
    {
        CurrentTarget = ClosestTarget;
    }
    else
    {
        CurrentTarget = nullptr;
    }
}


void ALA_TurretEnemy::FireProjectile()
{
    if (CurrentTarget && ProjectileClass && TurretHead)
    {
        float TurretHeightOffset = 80.0f; // 머리 높이 오프셋 통일
        FVector BaseLocation = GetActorLocation();
        FVector UpVector = GetActorUpVector();

        //  실제 터렛의 눈(포구) 월드 좌표 계산
        FVector RealEyeLocation = BaseLocation + (UpVector * TurretHeightOffset);

        FVector Forward = TurretHead->GetForwardVector();
        FVector ToTarget = (CurrentTarget->GetActorLocation() - RealEyeLocation).GetSafeNormal(); // 기준점 보정

        float AngleDot = FVector::DotProduct(Forward, ToTarget);

        // 정면 사잇각 검사 (고개가 완전히 안 돌아갔으면 사격 대기)
        if (AngleDot < 0.96f)
        {
            return;
        }

        FVector SpawnLocation = RealEyeLocation + (Forward * 80.0f);
        FRotator SpawnRotation = TurretHead->GetComponentRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this; // 안 쓰는 AI 컨트롤러 의존성 제거, 자기 자신 등록

        GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

        UE_LOG(LogTemp, Log, TEXT("[터렛 사격] 타겟 저격 발사 성공!"));
    }
}
bool ALA_TurretEnemy::CheckLineOfSight(AActor* TargetActor)
{
    if (!TargetActor || !GetWorld()) return false;

    FHitResult HitResult;

    // [핵심 수정] 가짜 바닥 좌표(GetComponentLocation) 대신,
    // 액터 바닥 위치에서 80 유닛만큼 제대로 들어 올린 '진짜 머리 높이'에서 레이저를 쏩니다!
    float TurretHeightOffset = 80.0f;
    FVector StartLocation = GetActorLocation() + (GetActorUpVector() * TurretHeightOffset);
    FVector EndLocation = TargetActor->GetActorLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(TargetActor);

    // Visibility 채널 레이트레이스 실행
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

    // [디버깅용 시각화] 포탑 눈에서 타겟까지 레이저 광선을 에디터 화면에 그려줍니다.

    return !bHit;
}

void ALA_TurretEnemy::SwitchTeam(FGameplayTag NewTeamTag)
{
    // [수정] UpdateTeamTag 부모 함수 호출 시 컴파일 에러가 난다면,
    // 터렛 내부 컨테이너를 직접 갱신하는 정석 방식으로 안전장치를 겁니다.
    CharacterTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));
    CharacterTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")));
    CharacterTags.AddTag(NewTeamTag);

    // 타겟 초기화
    CurrentTarget = nullptr;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
    }
}

float ALA_TurretEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = 0.0f;
    if (HealthComp)
    {
        ActualDamage = HealthComp->TakeDamage(DamageAmount, false);
        if (HealthWidgetComp) HealthWidgetComp->SetVisibility(true);

        // =================================================================
        // 🎯 [사운드 강제 직통 버그 수정]
        // =================================================================
        if (ActualDamage > 0.0f)
        {
            // 1. 부모가 물려준 오리지널 HitSound 변수명을 정확히 조준합니다.
            // (만약 부모 헤더의 변수명이 HitSound가 맞다면 이 코드가 작동합니다)
            if (HitSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
            }
            // 2. 만약 위 코드로도 소리가 안 난다면, 무기 베이스가 대미지를 가할 때
            // 꽂아준 DamageCauser(무기)나 EventInstigator(플레이어)의 사운드를 빌려 쓰거나,
            // 터렛 자체에 변수를 바인딩해야 합니다.
        }

        if (HealthComp->GetCurrentHealth() <= 0.0f)
        {
            Die();
            return ActualDamage;
        }
    }
    else
    {
        // 컴포넌트가 없을 땐 부모 함수를 타므로 부모의 사운드가 재생됩니다.
        ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    }

    // 대미지 텍스트 출력 로직...
    if (ActualDamage > 0.0f)
    {
        AccumulatedDamage += ActualDamage;
        if (!GetWorldTimerManager().IsTimerActive(DamageDisplayTimer))
        {
            GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_TurretEnemy::ExecuteShowDamageText, 0.05f, false);
        }
    }

    return ActualDamage;
}
void ALA_TurretEnemy::Die()
{
    // 부모의 Die() 기능을 먼저 실행시켜 공통 사망 처리(bIsDead = true 등)를 수행합니다.
    Super::Die();

    // 적 사망 시 GameMode에 전달
    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyEnemyKilled Called"));
        LA_GameMode->NotifyEnemyKilled(this);
    }

    //  터렛 고유의 타이머 및 기능 정지
    GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
    CurrentTarget = nullptr;
    PrimaryActorTick.SetTickFunctionEnable(false);

    //  터렛 고유의 폭발 이펙트 스폰
    if (DeathExplosionEffect && TurretHead)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathExplosionEffect, TurretHead->GetComponentLocation(), TurretHead->GetComponentRotation());
    }

    // 콜리전 비활성화
    if (TurretHead)
    {
        TurretHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 레벨에서 제거
    SetLifeSpan(2.0f);
}

void ALA_TurretEnemy::OnHealthChangedCallback(float CurrentHP, float MaxHP)
{
    CurrentHealth = CurrentHP;
    MaxHealth = MaxHP;

    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(CurrentHP);
    }

    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        Die();
    }
}

void ALA_TurretEnemy::ExecuteShowDamageText()
{
    if (!DamageTextClass || AccumulatedDamage <= 0.0f) return;

    ULA_EnemyDamageTextWidget* DamageWidget = CreateWidget<ULA_EnemyDamageTextWidget>(GetWorld(), DamageTextClass);
    if (DamageWidget)
    {
        DamageWidget->SetDamageValue(AccumulatedDamage);
        DamageWidget->AddToViewport();

        FVector WorldLocation = GetActorLocation() + FVector(0, 0, 100.0f);
        FVector2D ScreenPosition;

        if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetWorld()->GetFirstPlayerController(), WorldLocation, ScreenPosition, true))
        {
            DamageWidget->SetRenderTranslation(ScreenPosition);
        }

        ActiveDamageWidgets.Add(DamageWidget);

        TWeakObjectPtr<ALA_TurretEnemy> WeakSelf(this);
        TWeakObjectPtr<ULA_EnemyDamageTextWidget> WeakWidget(DamageWidget);

        FTimerHandle RemoveTimer;
        GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [WeakSelf, WeakWidget]()
            {
                if (WeakWidget.IsValid())
                {
                    WeakWidget->RemoveFromParent();
                    if (WeakSelf.IsValid())
                    {
                        WeakSelf->ActiveDamageWidgets.Remove(WeakWidget.Get());
                    }
                }
            }, 0.5f, false);
    }
    AccumulatedDamage = 0.0f;
}
