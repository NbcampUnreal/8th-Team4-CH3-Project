#include "Character/Enemy/LA_EnemyCharacter.h"
#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "Animation/AnimMontage.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/LA_EnemyDamageTextWidget.h"
#include "UI/LA_EnemyHealthWidget.h"
#include "Character/Player/Component/LA_HealthComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

ALA_EnemyCharacter::ALA_EnemyCharacter()
{
	AIControllerClass = ALA_EnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 80.0f;
	CurrentHealth = MaxHealth;

	MaxShield = 30.0f;
	CurrentShield = MaxShield;

	AttackPower = 15.0f;
	Defense = 3.0f;

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (EnemyTag.IsValid())
    {
        GameplayTags.AddTag(EnemyTag);
    }

    // 체력 컴포넌트 부착
    HealthWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
    HealthWidgetComp->SetupAttachment(RootComponent);
    HealthWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HealthWidgetComp->SetVisibility(false);             
}

void ALA_EnemyCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(GameplayTags);
}

float ALA_EnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    // 이미 죽었다면 무시
    if (bIsDead) return 0.0f;

    // 기본 대미지 계산
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 데이터 갱신 및 델리게이트 호출
    if (HealthComp)
    {
        HealthComp->TakeDamage(ActualDamage);
    }

    // 대미지 텍스트 타이머
    AccumulatedDamage += ActualDamage;
    GetWorld()->GetTimerManager().SetTimer(DamageDisplayTimer, this, &ALA_EnemyCharacter::ExecuteShowDamageText, 0.05f, false);

    // 5. 피격/사망 처리 분기
    if (bIsDead)
    {
        // 죽었을 때 로직
         HealthWidgetComp->SetVisibility(false);
    }
    else
    {
        // 살았을 때만 피격 애니메이션과 체력바 표시
        if (HealthWidgetComp)
        {
            HealthWidgetComp->SetVisibility(true);
        }

        if (ActualDamage > 0.0f && HitMontage)
        {
            PlayAnimMontage(HitMontage);
        }
    }

    return ActualDamage;
}


void ALA_EnemyCharacter::ExecuteShowDamageText()
{
    if (!DamageTextClass || AccumulatedDamage <= 0.0f) return;

    // 대미지 표시 위젯 생성
    ULA_EnemyDamageTextWidget* DamageWidget = CreateWidget<ULA_EnemyDamageTextWidget>(GetWorld(), DamageTextClass);

    if (DamageWidget)
    {
        // 화면에 대미지 위젯 표시
        DamageWidget->SetDamageValue(AccumulatedDamage);
        DamageWidget->AddToViewport();

        FVector WorldLocation = GetActorLocation() + FVector(0, 0, 100.0f);
        FVector2D ScreenPosition;

        if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
            GetWorld()->GetFirstPlayerController(), WorldLocation, ScreenPosition, true))
        {
            DamageWidget->SetRenderTranslation(ScreenPosition);
        }

        ActiveDamageWidgets.Add(DamageWidget);
        FTimerHandle RemoveTimer;

        // 0.5초 후 제거
        GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this, DamageWidget]()
            {
                if (DamageWidget)
                {
                    DamageWidget->RemoveFromParent();
                    ActiveDamageWidgets.Remove(DamageWidget);
                }
            }, 0.5f, false);
    }

    AccumulatedDamage = 0.0f;
}

void ALA_EnemyCharacter::Die()
{
	if (bIsDead) return;

    // 사망 즉시 체력바 숨기기
    if (HealthWidgetComp)
    {
        HealthWidgetComp->SetVisibility(false);
    }

	// 부모 클래스의 사망 로직 실행
	Super::Die();

	// 사망 애니메이션 재생
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	// 사망 시 이동 및 회전 완전 정지
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	SetLifeSpan(5.0f);
}


void ALA_EnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComp && HealthWidgetComp)
    {
        // 위젯 인스턴스 가져오기
        if (ULA_EnemyHealthWidget* HealthBar = Cast<ULA_EnemyHealthWidget>(HealthWidgetComp->GetUserWidgetObject()))
        {
            // 바인딩
            HealthComp->OnHealthChanged.AddUObject(HealthBar, &ULA_EnemyHealthWidget::UpdateHealthBar);

            // 초기화
            HealthBar->UpdateHealthBar(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
        }
    }

    if (!HealthComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] HealthComponent를 찾을 수 없습니다!"), *GetName());
    }

}

void ALA_EnemyCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // 컴포넌트 찾기
    HealthComp = FindComponentByClass<ULA_HealthComponent>();

}

