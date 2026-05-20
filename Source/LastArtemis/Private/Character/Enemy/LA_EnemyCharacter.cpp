#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "Animation/AnimMontage.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/LA_EnemyDamageTextWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/LA_EnemyHealthWidget.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/LA_GameModeBase.h"

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

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    }

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

    bIsAttacking = false;
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
            //[최종 수정] 타이머의 여유 시간(+0.5f)을 완전히 제거하고 순수 'Duration'으로만 맞춥니다!
            // 애니메이션이 눈으로 보기에 딱 끝나는 그 타이밍에 정확하게 bIsAttacking을 false로 밀어줍니다.
            GetWorld()->GetTimerManager().SetTimer(
                AttackTimerHandle,
                [this]()
                {
                    bIsAttacking = false;
                    // [안전장치] 혹시 비헤이비어 트리가 멈춰있을지 모르니,
                    // 공격 상태가 풀리는 순간에 다음 프레임 연산을 강제로 깨우는 안전벨트입니다.
                    PrimaryActorTick.SetTickFunctionEnable(false);
                },
                Duration, // 정확히 애니메이션 실제 길이만큼만 대기!
                false
            );
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

            // 🎯 OwnerComp와 ActionNode를 함께 람다 캡처로 넘겨줍니다.
            GetWorld()->GetTimerManager().SetTimer(
                AttackTimerHandle,
                [this, OwnerComp, ActionNode]()
                {
                    bIsAttacking = false;

                    if (OwnerComp && ActionNode)
                    {
                        UBehaviorTreeComponent* BTComp = const_cast<UBehaviorTreeComponent*>(OwnerComp);
                        if (BTComp)
                        {
                            // 💥 [수정] 불완전한 GetActiveNode() 대신 확실한 테스크 노드 주소를 직접 꽂아줍니다!
                            BTComp->OnTaskFinished(ActionNode, EBTNodeResult::Succeeded);
                        }
                    }
                },
                Duration,
                false
            );
        }
        else
        {
            bIsAttacking = false;
        }
    }
}

void ALA_EnemyCharacter::EnemyMeleeAttackCheck()
{
    // [수정] 시작 지점을 에너미 중심에서 약간 뒤나 정확한 중심축으로 잡고, 사거리를 확실하게 늘려 헛방을 방지합니다.
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation + GetActorForwardVector() * (MeleeAttackRange + 60.0f);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    FHitResult HitResult;

    // [수정] 플레이어가 무조건 반응할 수밖에 없는 'ECC_Visibility' 채널로 변경합니다.
    // 플레이어 카메라나 화면 렌더링용 채널이기 때문에 어떤 액터든 기본적으로 블록(Block)이 켜져 있습니다.
    bool bHit = UKismetSystemLibrary::SphereTraceSingle(
         GetWorld(), StartLocation, EndLocation, MeleeAttackRadius + 10.0f,
         UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore,
         EDrawDebugTrace::ForDuration, HitResult, true);

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();

        // 1. 동료 에너미 차단
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

        // [수정] 타격 대상이 아군 AI인지 검증하는 방어막 확장
        bool bIsAlly = false;
        if (TargetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))) ||
            HitActor->ActorHasTag(FName("Team.Ally")) ||
            HitActor->GetName().Contains(TEXT("Player")) ||
            HitActor->GetName().Contains(TEXT("Ally")) ||
            HitActor->GetClass()->GetName().Contains(TEXT("Ally")))
        {
            bIsAlly = true;
        }

        // 확실하게 아군 판정이 나면 대미지 꽂기
        if (bIsAlly)
        {
            float HeavyDamage = 50.0f;
            UGameplayStatics::ApplyDamage(HitActor, HeavyDamage, GetController(), this, nullptr);

            UE_LOG(LogTemp, Error, TEXT("🔥 [적 공격 성공] 아군 AI(또는 플레이어) 타격 완료! 대상: %s 🔥"), *HitActor->GetName());
        }
    }
    else
    {
        // 아예 안 맞았을 때 로그 (디버깅용)
        UE_LOG(LogTemp, Log, TEXT("💨 에너미가 칼을 휘둘렀으나 사거리가 닿지 않거나 콜리전 채널이 빗나감."));
    }
}

float ALA_EnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                     class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.0f) return 0.0f;

    AccumulatedDamage += ActualDamage;
    if (!GetWorldTimerManager().IsTimerActive(DamageDisplayTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_EnemyCharacter::ExecuteShowDamageText, 0.05f, false);
    }

    if (CurrentHealth > 0.0f)
    {
        if (HealthWidgetComp) HealthWidgetComp->SetVisibility(true);

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

    // 적 사망 시 GameMode에 전달
    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyEnemyKilled Called"));
        LA_GameMode->NotifyEnemyKilled(this);
    }

    if (DeathMontage) PlayAnimMontage(DeathMontage);

    if (GetCharacterMovement())
    {
       GetCharacterMovement()->StopMovementImmediately(); // 즉시 정지 추가
       GetCharacterMovement()->DisableMovement();
    }

    SetLifeSpan(5.0f);
}
