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
    // [수정] 만약 부모(BaseCharacter)의 TakeDamage 내부에 이미
    // "실드 차감 -> 체력 차감 -> CurrentHealth <= 0 일 때 Die() 호출" 로직이 완벽하게 짜여 있다면?
    // 터렛 C++ 파일에서는 TakeDamage 함수를 통째로 지워버려도 자동으로 부모 코드가 작동합니다!

    // 하지만 터렛만 방어력 연산을 다르게 하거나 로그를 찍어야 한다면 아래처럼 부모 함수를 먼저 호출해 줍니다.
    if (bIsDead) return 0.0f;

    // 부모의 데미지 공식을 실행시켜 부모 변수(CurrentHealth)를 알아서 깎게 만듭니다.
    float ActualDamage = 0.0f;
    if (HealthComp)
    {
        ActualDamage = HealthComp->TakeDamage(DamageAmount, false);
        if (HealthWidgetComp) HealthWidgetComp->SetVisibility(true);
    }
    else
    {
        ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    }

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
