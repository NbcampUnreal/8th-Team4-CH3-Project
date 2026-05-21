#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "Character/LA_BaseCharacter.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameMode/LA_GameModeBase.h"
#include "Kismet/GameplayStatics.h" 

ALA_BossCharacter::ALA_BossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 💡 부모 변수(MaxHealth, CurrentHealth)가 있다면 그대로 할당됩니다.
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;

    MaxShield = 100.0f;
    CurrentShield = MaxShield;

    CurrentPhase = EBossPhase::Phase1;
    bIsDead = false;

    if (GetCharacterMovement())
    {
        // 보스가 제자리에 고정되어 패턴을 쓰는 붙박이형이라면 잘 하신 세팅입니다!
        // 만약 나중에 뛰어다녀야 한다면 이 줄을 지우거나 MOVE_Walking으로 바꿔야 합니다.
        GetCharacterMovement()->SetMovementMode(MOVE_None);
    }
}

void ALA_BossCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 🎯 [태그 봉인 치트키] 블루프린트 세팅에 상관없이 게임 시작 시 무조건 적군으로 세팅!
    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));
    if (!CharacterTags.HasTag(EnemyTag))
    {
        CharacterTags.AddTag(EnemyTag);
    }
}

float ALA_BossCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    // 🎯 [중요 수술] 이중 차감을 막기 위해 Super 호출을 가두거나 제어해야 합니다.
    // 만약 부모(BaseCharacter)의 TakeDamage에 "체력을 그냥 깎는 로직"만 들어있다면,
    // 부모 코드를 실행하지 않고 보스 고유의 [쉴드 -> 체력] 연산만 깔끔하게 돌린 뒤 자식 단에서 정산합니다.

    if (DamageAmount <= 0.0f) return 0.0f;

    float ActualDamage = DamageAmount; // 방어력 공식이 부모에게 있다면 Super 대신 여기서 직접 계산하거나 원본을 씁니다.

    // 🛡️ 쉴드 및 체력 정산 (이중 차감 없이 정직하게 1번만 계산)
    if (CurrentShield > 0.0f)
    {
        CurrentShield -= ActualDamage;
        if (CurrentShield < 0.0f)
        {
            CurrentHealth += CurrentShield; // 음수 값만큼 체력 차감
            CurrentShield = 0.0f;
        }
    }
    else
    {
        CurrentHealth -= ActualDamage;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("👹 [보스 피격] 데미지: %f | 쉴드: %f | 체력: %f / %f"), ActualDamage, CurrentShield, CurrentHealth, MaxHealth);

    // 사망 처리
    if (CurrentHealth <= 0.0f)
    {
        Die();
        return ActualDamage;
    }

    // 페이즈 전환 체크 (사망 안 했을 때만)
    CheckPhaseTransition();

    // 🎯 [센스 옵션] 페이즈가 막 바뀌는 순간에는 피격 모션(HitMontage)이 재생되어
    // 광폭화/페이즈 변환 연출이 끊기는 걸 방지하는 것이 자연스럽습니다.
    if (HitMontage)
    {
        PlayAnimMontage(HitMontage);
    }

    // 부모 클래스의 데미지 관련 알림(인터페이스, UI 브로드캐스트 등)이 작동해야 한다면
    // 체력을 다 깎아놓은 최종 상태에서 0의 대미지로 찔러넣어 알림만 가게 우회할 수도 있습니다.
    Super::TakeDamage(0.0f, DamageEvent, EventInstigator, DamageCauser);

    return ActualDamage;
}

void ALA_BossCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    UE_LOG(LogTemp, Error, TEXT("💀 [보스 사망] 격전 끝에 보스가 완벽하게 처치되었습니다! 💀"));

    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    // 콜리전 비활성화
    UCapsuleComponent* MyCapsule = GetCapsuleComponent();
    if (MyCapsule)
    {
        MyCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 무브먼트 컴포넌트 비활성화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // AI 컨트롤러 작동 중지 및 분리 (패턴 정지)
    if (GetController())
    {
        GetController()->StopMovement();
        GetController()->UnPossess();
    }

    // 시체가 맵에 머무르는 시간 (10초 뒤 소멸)
    SetLifeSpan(10.0f);

    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyBossKilled Called"));
        LA_GameMode->NotifyEnemyKilled(this);
    }
}

void ALA_BossCharacter::CheckPhaseTransition()
{
    if (bIsDead) return;

    float HealthPercent = CurrentHealth / MaxHealth;

    if (HealthPercent <= 0.33f && CurrentPhase != EBossPhase::Phase3)
    {
        CurrentPhase = EBossPhase::Phase3;
        UE_LOG(LogTemp, Error, TEXT("❗ [보스 페이즈 변경] >>> 폭주: PHASE 3 진입 <<< ❗"));

        // 💡 팁: 여기에 페이즈 3 진입 시 쓸 광폭화 몽타주나 이펙트 코드를 심으면 좋습니다.
    }
    else if (HealthPercent <= 0.66f && CurrentPhase == EBossPhase::Phase1)
    {
        CurrentPhase = EBossPhase::Phase2;
        UE_LOG(LogTemp, Warning, TEXT("⚡ [보스 페이즈 변경] >>> PHASE 2 진입 <<< ⚡"));
    }
}
