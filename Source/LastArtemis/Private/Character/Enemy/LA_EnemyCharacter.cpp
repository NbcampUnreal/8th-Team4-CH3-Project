#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "Animation/AnimMontage.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/LA_EnemyDamageTextWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LA_EnemyHealthWidget.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

ALA_EnemyCharacter::ALA_EnemyCharacter()
{
    AIControllerClass = ALA_EnemyController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    PrimaryActorTick.bCanEverTick = false;

    MaxHealth = 80.0f;
    CurrentHealth = MaxHealth;
    MaxShield = 30.0f;
    CurrentShield = MaxShield;
    AttackPower = 15.0f;
    Defense = 3.0f;

    // 팀 태그 설정
    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        GameplayTags.AddTag(EnemyTag);
    }

    HealthWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
    HealthWidgetComp->SetupAttachment(RootComponent);
    HealthWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HealthWidgetComp->SetVisibility(false);
}

void ALA_EnemyCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    HealthComp = FindComponentByClass<ULA_HealthComponent>();
}

void ALA_EnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComp && HealthWidgetComp)
    {
        if (ULA_EnemyHealthWidget* HealthBar = Cast<ULA_EnemyHealthWidget>(HealthWidgetComp->GetUserWidgetObject()))
        {
            HealthComp->OnHealthChanged.AddUObject(HealthBar, &ULA_EnemyHealthWidget::UpdateHealthBar);
            HealthBar->UpdateHealthBar(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
        }
    }
}

void ALA_EnemyCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(GameplayTags);
}

void ALA_EnemyCharacter::EnemyMeleeAttackCheck()
{
    // 공격 판정 로직 (기존 유지)
    FVector StartLocation = GetActorLocation() + GetActorForwardVector() * 40.f;
    FVector EndLocation = StartLocation + GetActorForwardVector() * MeleeAttackRange;

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    FHitResult HitResult;

    bool bHit = UKismetSystemLibrary::SphereTraceSingle(
         GetWorld(), StartLocation, EndLocation, MeleeAttackRadius,
         UEngineTypes::ConvertToTraceType(ECC_Pawn), false, ActorsToIgnore,
         EDrawDebugTrace::ForDuration, HitResult, true
     );

    if (bHit && HitResult.GetActor())
    {
        UGameplayStatics::ApplyDamage(HitResult.GetActor(), AttackPower, GetController(), this, nullptr);
    }
}

float ALA_EnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                     class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    // 1. 부모의 데미지 계산 로직 호출
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (HealthComp)
    {
        HealthComp->TakeDamage(ActualDamage);
    }

    // 2. 데미지 텍스트 누적 표시
    AccumulatedDamage += ActualDamage;
    if (!GetWorldTimerManager().IsTimerActive(DamageDisplayTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_EnemyCharacter::ExecuteShowDamageText, 0.05f, false);
    }

    // 3. 상태 피드백 (사망 시 처리는 Die()에서 하므로 여기선 피격 리액션만)
    if (!bIsDead && ActualDamage > 0.0f)
    {
        if (HealthWidgetComp) HealthWidgetComp->SetVisibility(true);

        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
        {
            // [정리] 특정 몽타주가 아닌 '현재 재생 중인 모든 몽타주'를 정지하여 확실히 경직시킴
            AnimInstance->Montage_Stop(0.2f);
        }

        if (HitMontage) PlayAnimMontage(HitMontage);
    }

    return ActualDamage;
}

void ALA_EnemyCharacter::ExecuteShowDamageText()
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

        // [안전성] TWeakObjectPtr를 사용하여 액터 파괴 시 안전성 확보
        TWeakObjectPtr<ALA_EnemyCharacter> WeakSelf(this);
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

void ALA_EnemyCharacter::Die()
{
    if (bIsDead) return;

    if (HealthWidgetComp) HealthWidgetComp->SetVisibility(false);

    // 데미지 텍스트 타이머 정리
    GetWorldTimerManager().ClearTimer(DamageDisplayTimer);

    Super::Die(); // 여기서 bIsDead가 true가 됨

    if (DeathMontage) PlayAnimMontage(DeathMontage);

    if (GetCharacterMovement())
    {
       GetCharacterMovement()->StopMovementImmediately(); // 즉시 정지 추가
       GetCharacterMovement()->DisableMovement();
    }

    SetLifeSpan(5.0f);
}
