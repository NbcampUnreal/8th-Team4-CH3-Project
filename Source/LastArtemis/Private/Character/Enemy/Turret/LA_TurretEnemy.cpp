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

// Sets default values
ALA_TurretEnemy::ALA_TurretEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    TurretHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretHead"));
    TurretHead->SetupAttachment(RootComponent);

    // 💡 [참고] MaxHealth, Defense 등의 스탯은 컴포넌트에서 관리하므로 여기서 지웁니다.
    bIsDead = false;
    DetectionRange = 2000.0f;
    RotationSpeed = 5.0f;
    FireRate = 1.5f;

    CurrentTarget = nullptr;

    CharacterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));


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
    Super::BeginPlay(); // 🎯 부모의 BeginPlay가 돌며 중앙 HealthComponent 생성 완료

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (!CharacterTags.HasTag(EnemyTag))
    {
        CharacterTags.AddTag(EnemyTag);
    }
    if (!ActorHasTag(FName("Team.Enemy")))
    {
        Tags.Add(FName("Team.Enemy"));
    }

    GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ALA_TurretEnemy::FireProjectile, FireRate, true);

    // 🎯 [수정] 델리게이트 릴레이 바인딩 (터렛 본체의 피격 콜백 연결)
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddUObject(this, &ALA_TurretEnemy::OnHealthChangedCallback);
    }

    // 🎯 [핵심 보안 기믹] 위젯 인스턴스가 안전하게 초기화될 수 있도록 1프레임 뒤(또는 즉시) 타이밍 우회 바인딩
    if (HealthWidgetComp)
    {
        // 안전하게 다음 틱이나 렌더링 준비 단계에서 위젯 오브젝트를 꺼냅니다.
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (HealthWidgetComp && HealthComponent)
            {
                if (ULA_EnemyHealthWidget* HealthBar = Cast<ULA_EnemyHealthWidget>(HealthWidgetComp->GetUserWidgetObject()))
                {
                    // 컴포넌트 체력 변경 이벤트를 위젯의 UpdateHealthBar 함수와 다이렉트로 결합!
                    HealthComponent->OnHealthChanged.AddUObject(HealthBar, &ULA_EnemyHealthWidget::UpdateHealthBar);

                    // 초기 체력 바 레이아웃 갱신
                    HealthBar->UpdateHealthBar(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
                }
            }
        });
    }
}

float ALA_TurretEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    // 부모의 TakeDamage를 호출하여 컴포넌트 피 정산 및 피격음을 안전하게 수신합니다.
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.0f) return 0.0f;

    // 🔥 [버그 킬러] 평소엔 숨겨져 있던 터렛 전용 체력 바 위젯을 타격당하는 순간 즉시 뷰포트에 오픈!
    if (HealthWidgetComp)
    {
        HealthWidgetComp->SetVisibility(true);
    }

    // 연사 무기 대비 누적 대미지 텍스트 팝업 프로세서 가동
    AccumulatedDamage += ActualDamage;
    if (!GetWorldTimerManager().IsTimerActive(DamageDisplayTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_TurretEnemy::ExecuteShowDamageText, 0.05f, false);
    }

    return ActualDamage;
}
// 💡 중간 연산 함수들 (Tick, FindTarget, FireProjectile, CheckLineOfSight, SwitchTeam)은 완벽하므로 기존 로직 그대로 유지됩니다.

void ALA_TurretEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FindTarget();

    if (CurrentTarget && TurretHead)
    {
        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(TurretHead->GetComponentLocation(), CurrentTarget->GetActorLocation());
        TargetRot.Roll = 0.0f;
        FRotator SmoothedRot = FMath::RInterpTo(TurretHead->GetComponentRotation(), TargetRot, DeltaTime, RotationSpeed);
        TurretHead->SetWorldRotation(SmoothedRot);
    }
}

void ALA_TurretEnemy::FindTarget()
{
    if (!GetWorld() || !HealthComponent) return;

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

        if (bIsOriginallyEnemy)
        {
            if (!bIsTargetAlly) continue;
        }
        else
        {
            if (!bIsTargetEnemy) continue;
        }

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

    if (ClosestTarget) CurrentTarget = ClosestTarget;
    else CurrentTarget = nullptr;
}

void ALA_TurretEnemy::FireProjectile()
{
    if (CurrentTarget && ProjectileClass && TurretHead)
    {
        float TurretHeightOffset = 80.0f;
        FVector RealEyeLocation = GetActorLocation() + (GetActorUpVector() * TurretHeightOffset);
        FVector Forward = TurretHead->GetForwardVector();
        FVector ToTarget = (CurrentTarget->GetActorLocation() - RealEyeLocation).GetSafeNormal();

        float AngleDot = FVector::DotProduct(Forward, ToTarget);
        if (AngleDot < 0.96f) return;

        FVector SpawnLocation = RealEyeLocation + (Forward * 80.0f);
        FRotator SpawnRotation = TurretHead->GetComponentRotation();

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
    float TurretHeightOffset = 80.0f;
    FVector StartLocation = GetActorLocation() + (GetActorUpVector() * TurretHeightOffset);
    FVector EndLocation = TargetActor->GetActorLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(TargetActor);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);
    return !bHit;
}

void ALA_TurretEnemy::SwitchTeam(FGameplayTag NewTeamTag)
{
    CharacterTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy")));
    CharacterTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")));
    CharacterTags.AddTag(NewTeamTag);

    CurrentTarget = nullptr;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC) AIC->StopMovement();
}

// 🎯 [대수술 완료 부위] 자식 고유의 TakeDamage 구현을 완전히 날리거나 최소화하여 부모의 파이프라인으로 태워보냅니다.

void ALA_TurretEnemy::Die()
{
    // 🎯 2. 부모의 Die()를 먼저 호출하여 공통 플래그 및 캡슐 콜리전 해제 처리 수행
    Super::Die();

    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        LA_GameMode->NotifyEnemyKilled(this);
    }

    GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
    CurrentTarget = nullptr;
    PrimaryActorTick.SetTickFunctionEnable(false);

    if (DeathExplosionEffect && TurretHead)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathExplosionEffect, TurretHead->GetComponentLocation(), TurretHead->GetComponentRotation());
    }

    if (TurretHead)
    {
        TurretHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    SetLifeSpan(2.0f);
}

void ALA_TurretEnemy::OnHealthChangedCallback(float CurrentHP, float MaxHP)
{
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(CurrentHP);
    }
    if (CurrentHP <= 0.0f && !bIsDead)
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
