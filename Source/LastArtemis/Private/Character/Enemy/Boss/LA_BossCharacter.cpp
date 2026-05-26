#include "Character/Enemy/Boss/LA_BossCharacter.h"
#include "Character/LA_BaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "NiagaraFunctionLibrary.h"
#include "GameMode/LA_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ALA_BossCharacter::ALA_BossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // [스태틱 메시 컴포넌트 장착]
    BossStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossStaticMesh"));

    if (GetCapsuleComponent())
    {
        BossStaticMesh->SetupAttachment(GetCapsuleComponent());
    }

    // 💡 참고: 이제 MaxHealth, CurrentShield 등은 부모가 들고 있는 HealthComponent가 제어합니다.
    // 수치 설정은 생성자 대신 아래 PostInitializeComponents 단계에서 오버라이딩해 줍니다.
    CurrentPhase = EBossPhase::Phase1;
    bIsDead = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_None);
    }
}

void ALA_BossCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // 🎯 1. [보스 전용 수치 강제 주입]
    // 부모의 생성 주기가 끝난 후 보스 기획에 맞춰 컴포넌트 내부 스탯을 최종 갱신합니다.
    if (HealthComponent)
    {
        HealthComponent->SetMaxHealth(500.0f);
        HealthComponent->SetCurrentHealth(500.0f);
        HealthComponent->SetMaxShield(100.0f);
        HealthComponent->SetCurrentShield(100.0f);
        HealthComponent->SetDefense(5.0f); // 방어력 수치 세팅
    }
}

void ALA_BossCharacter::BeginPlay()
{
    Super::BeginPlay();

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));

    if (!CharacterTags.HasTag(EnemyTag))
    {
        CharacterTags.AddTag(EnemyTag);
    }

    if (!ActorHasTag(FName("Team.Enemy")))
    {
        Tags.Add(FName("Team.Enemy"));
    }
}

float ALA_BossCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead || DamageAmount <= 0.0f) return 0.0f;

    // 🎯 2. [대미지 및 사운드 파이프라인 일원화]
    // 자식에서 직접 피를 깎지 않고 부모의 Super::TakeDamage를 정석대로 실행시킵니다.
    // 이 한 줄로 부모 내부의 HealthComponent가 실드/피 정산과 블루프린트 UI 알림을 한 번에 처리하고,
    // 부모에 등록된 찰진 피격 사운드(HitSound)까지 100% 정상 출력해 줍니다.
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.0f) return 0.0f;

    // [이펙트 연출] 기계형 보스 고유의 피격 스파크 니아가라 스폰
    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation());
    }

    // 🎯 3. 사망 처리는 부모 단에서 자동으로 감지해 Die()를 부르지만,
    // 보스 전용 소멸 연출을 매끄럽게 이어가기 위해 더블 가드를 세워둡니다.
    if (HealthComponent && HealthComponent->GetCurrentHealth() <= 0.0f)
    {
        return ActualDamage;
    }

    // 🎯 4. 페이즈 전환 체크 (컴포넌트의 실시간 체력 주소를 넘겨서 계산)
    CheckPhaseTransition();

    return ActualDamage;
}

void ALA_BossCharacter::Die()
{
    if (bIsDead) return;

    // 🎯 5. 부모의 Die()를 먼저 호출하여 공통 사망 플래그(bIsDead = true)와 캡슐 콜리전 해제 처리 수행
    Super::Die();

    // [사망 연출] 로봇 코어 폭발 니아가라 스폰
    if (DeathExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DeathExplosionEffect, GetActorLocation());
    }

    // 보스 고유의 스태틱 메시 콜리전도 완벽하게 꺼줍니다.
    if (BossStaticMesh)
    {
        BossStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (GetCharacterMovement()) GetCharacterMovement()->DisableMovement();
    if (GetController())
    {
        GetController()->StopMovement();
        GetController()->UnPossess();
    }

    SetLifeSpan(5.0f);

    if (ALA_GameModeBase* LA_GameMode = Cast<ALA_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        LA_GameMode->NotifyEnemyKilled(this);
    }
}

void ALA_BossCharacter::CheckPhaseTransition()
{
    if (bIsDead || !HealthComponent) return;

    // 🎯 6. 변수가 날아간 자리를 컴포넌트의 인라인 Getter 함수로 싱크 교정합니다.
    float HealthPercent = HealthComponent->GetHealthPercent();

    if (HealthPercent <= 0.33f && CurrentPhase != EBossPhase::Phase3)
    {
        CurrentPhase = EBossPhase::Phase3;
        UE_LOG(LogTemp, Error, TEXT("❗ [보스 페이즈 변경] >>> 폭주: PHASE 3 진입 <<< ❗"));
    }
    else if (HealthPercent <= 0.66f && CurrentPhase == EBossPhase::Phase1)
    {
        CurrentPhase = EBossPhase::Phase2;
        UE_LOG(LogTemp, Warning, TEXT("⚡ [보스 페이즈 변경] >>> PHASE 2 진입 <<< ⚡"));
    }
}
