#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/LA_EnemyHealthWidget.h"
#include "UI/LA_EnemyDamageTextWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/LA_GameModeBase.h"

ALA_EnemyCharacter::ALA_EnemyCharacter()
{
    AIControllerClass = ALA_EnemyController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    PrimaryActorTick.bCanEverTick = false;

    // 💡 [수정] 생성자 시점의 안전하지 않은 수치 세팅은 삭제하고 아래 PostInitializeComponents 단으로 완전히 통합 이주했습니다.

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    }

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        GameplayTags.AddTag(EnemyTag);
    }

    HealthWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
    HealthWidgetComp->SetupAttachment(RootComponent);
    HealthWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HealthWidgetComp->SetVisibility(false);

    bIsAttacking = false;
}

void ALA_EnemyCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // 🎯 부모가 물려준 헬스 컴포넌트 변수를 찾아서 주소를 명확히 바인딩합니다.
    HealthComponent = FindComponentByClass<ULA_HealthComponent>();

    // 🎯 일반 잡몹 기획 스탯 수치를 안전한 시점에 오버라이딩합니다.
    if (HealthComponent)
    {
        HealthComponent->SetMaxHealth(80.0f);
        HealthComponent->SetCurrentHealth(80.0f);
        HealthComponent->SetMaxShield(30.0f);
        HealthComponent->SetCurrentShield(30.0f);
        HealthComponent->SetAttackPower(15.0f);
        HealthComponent->SetDefense(3.0f);
    }
}

void ALA_EnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent && HealthWidgetComp)
    {
        if (ULA_EnemyHealthWidget* HealthBar = Cast<ULA_EnemyHealthWidget>(HealthWidgetComp->GetUserWidgetObject()))
        {
            HealthComponent->OnHealthChanged.AddUObject(HealthBar, &ULA_EnemyHealthWidget::UpdateHealthBar);
            HealthBar->UpdateHealthBar(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
        }
    }
}

void ALA_EnemyCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(GameplayTags);
}

void ALA_EnemyCharacter::PlayAttackMontage()
{
    if (bIsAttacking || bIsDead) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        bIsAttacking = true;
        float Duration = PlayAnimMontage(AttackMontage);

        if (Duration > 0.0f)
        {
            FTimerHandle AttackTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [this]()
                {
                    bIsAttacking = false;
                    PrimaryActorTick.SetTickFunctionEnable(false);
                }, Duration, false);
        }
        else
        {
            bIsAttacking = false;
        }
    }
}

void ALA_EnemyCharacter::PlayAttackMontageWithComp(UBehaviorTreeComponent* OwnerComp, const UBTTaskNode* ActionNode)
{
    if (bIsAttacking || bIsDead) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        bIsAttacking = true;
        float Duration = PlayAnimMontage(AttackMontage);

        if (Duration > 0.0f)
        {
            FTimerHandle AttackTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [this, OwnerComp, ActionNode]()
                {
                    bIsAttacking = false;
                    if (OwnerComp && ActionNode)
                    {
                        UBehaviorTreeComponent* BTComp = const_cast<UBehaviorTreeComponent*>(OwnerComp);
                        if (BTComp)
                        {
                            BTComp->OnTaskFinished(ActionNode, EBTNodeResult::Succeeded);
                        }
                    }
                }, Duration, false);
        }
        else
        {
            bIsAttacking = false;
        }
    }
}

void ALA_EnemyCharacter::EnemyMeleeAttackCheck()
{
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation + GetActorForwardVector() * (MeleeAttackRange + 60.0f);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    FHitResult HitResult;
    bool bHit = UKismetSystemLibrary::SphereTraceSingle(
         GetWorld(), StartLocation, EndLocation, MeleeAttackRadius + 10.0f,
         UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore,
         EDrawDebugTrace::ForDuration, HitResult, true);

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(HitActor);
        FGameplayTagContainer TargetTags;
        if (TagInterface)
        {
            TagInterface->GetOwnedGameplayTags(TargetTags);
        }

        if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Enemy"))) || HitActor->ActorHasTag(FName("Team.Enemy")))
        {
            return;
        }

        bool bIsAlly = false;
        if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))) ||
            HitActor->ActorHasTag(FName("Team.Ally")) ||
            HitActor->GetName().Contains(TEXT("Player")) ||
            HitActor->GetName().Contains(TEXT("Ally")) ||
            HitActor->GetClass()->GetName().Contains(TEXT("Ally")))
        {
            bIsAlly = true;
        }

        if (bIsAlly)
        {
            float HeavyDamage = 50.0f;
            UGameplayStatics::ApplyDamage(HitActor, HeavyDamage, GetController(), this, nullptr);
            UE_LOG(LogTemp, Error, TEXT("🔥 [적 공격 성공] 대상: %s 🔥"), *HitActor->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("💨 에너미 사거리 미달 혹은 콜리전 채널 빗나감."));
    }
}

float ALA_EnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                     class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    // 🎯 부모 클래스의 완성된 TakeDamage 파이프라인을 정석대로 탑승시킵니다.
    // 헬스 컴포넌트 정산 및 공통 'HitSound'가 여기서 완벽히 한 번에 흘러나옵니다!
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.0f) return 0.0f;

    if (HealthWidgetComp) HealthWidgetComp->SetVisibility(true);

    AccumulatedDamage += ActualDamage;

    if (!GetWorldTimerManager().IsTimerActive(DamageDisplayTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_EnemyCharacter::ExecuteShowDamageText, 0.05f, false);
    }

    // 피격 애니메이션 리액션 체크
    if (HealthComponent && HealthComponent->GetCurrentHealth() > 0.0f)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
        {
            if (HitMontage)
            {
                PlayAnimMontage(HitMontage);
            }
        }
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
    GetWorldTimerManager().ClearTimer(DamageDisplayTimer);

    // 부모의 Die()를 호출하여 콜리전 및 사망 처리 일원화
    Super::Die();

    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyEnemyKilled Called"));
        LA_GameMode->NotifyEnemyKilled(this);
    }

    if (DeathMontage) PlayAnimMontage(DeathMontage);

    if (GetCharacterMovement())
    {
       GetCharacterMovement()->StopMovementImmediately();
       GetCharacterMovement()->DisableMovement();
    }

    SetLifeSpan(5.0f);
}
