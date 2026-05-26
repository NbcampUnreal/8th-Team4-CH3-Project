// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/LA_BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ALA_BaseCharacter::ALA_BaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // 🎯 1. 부모 단에서 공통 헬스 컴포넌트 오브젝트 생성
    HealthComponent = CreateDefaultSubobject<ULA_HealthComponent>(TEXT("HealthComponent"));

    MaxContamination = 100.0f;
    Contamination = 0.0f;
    bIsDead = false;

    // 💡 참고: 기존에 헤더에 직접 선언되어 있던 MaxHealth, CurrentHealth, Shield, Defense 등의
    // 기본 수치들은 이제 컴포넌트가 전부 쥐고 관리하므로 생성자 초기화 라인에서 제거되었습니다.
    // 해당 디폴트 수치들은 블루프린트 에디터 내 HealthComponent 디테일 창에서 편하게 세팅하시면 됩니다!
}

void ALA_BaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        // 🎯 2. [자동 사망 체인] 컴포넌트에서 체력이 0이 되어 OnDeath 신호를 주면 본체의 Die()가 불리도록 바인딩
        HealthComponent->OnDeath.AddDynamic(this, &ALA_BaseCharacter::Die);

        // 🎯 3. [UI 싱크 연동] 컴포넌트 내부 수치가 바뀔 때 기존 블루프린트 딜리게이트가 릴레이 알림을 주도록 연결
        HealthComponent->OnHealthChanged.AddUObject(this, &ALA_BaseCharacter::HandleHealthChanged);
        HealthComponent->OnShieldChanged.AddUObject(this, &ALA_BaseCharacter::HandleShieldChanged);
    }
}

float ALA_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 본체 플래그 혹은 컴포넌트 단에서 이미 사망 판정이 났거나 대미지가 유효하지 않다면 스킵
    if (bIsDead || (HealthComponent && HealthComponent->IsDead()) || DamageAmount <= 0.0f) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 🎯 4. [대미지 위임] 복잡한 방어력 상쇄, 쉴드 흡수, 체력 정산 로직을 현묵님의 컴포넌트로 일원화합니다.
    if (HealthComponent)
    {
        ActualDamage = HealthComponent->TakeDamage(DamageAmount, false);

        // 🎯 5. [피격음 버그 영구 수정] 컴포넌트를 통과한 최종 실질 대미지가 들어갔을 때만 피격 사운드 재생
        if (ActualDamage > 0.0f && HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
        }
    }

    return ActualDamage;
}

void ALA_BaseCharacter::TakeDamageCustom(float DamageAmount)
{
    if (bIsDead || (HealthComponent && HealthComponent->IsDead()) || DamageAmount <= 0.0f) return;

    // 커스텀 대미지 요청 역시 헬스 컴포넌트 파이프라인으로 정확하게 관통시킵니다.
    if (HealthComponent)
    {
        float ActualDamage = HealthComponent->TakeDamage(DamageAmount, false);

        if (ActualDamage > 0.0f && HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
        }
    }
}

void ALA_BaseCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    if (GetCapsuleComponent())
    {
       GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 블루프린트 UI나 게임모드 시스템이 캐치할 수 있도록 기존 사망 이벤트 발송
    if (OnDeath.IsBound()) OnDeath.Broadcast();
}

// 🎯 6. [오염도 연동] 본체가 쥐고 있던 로직을 컴포넌트의 AddContamination 함수 호출로 우회 세팅
void ALA_BaseCharacter::IncreaseContamination(float Amount)
{
    if (HealthComponent)
    {
        HealthComponent->AddContamination(Amount);
    }
}

// 🎯 7. [오염 정화 연동] 컴포넌트 내부의 Decontaminate 함수 호출로 싱크 매칭
void ALA_BaseCharacter::Decontaminate(float Amount)
{
    if (HealthComponent)
    {
        HealthComponent->Decontaminate(Amount);
    }
}

// 🎯 8. [특수 쉴드 깎기 연동] 컴포넌트의 현재 실드 값을 안전하게 Set 해주는 방식으로 우회 교정
void ALA_BaseCharacter::ReduceShieldOnly(float Amount)
{
    if (HealthComponent)
    {
        float NewShield = FMath::Max(HealthComponent->GetCurrentShield() - Amount, 0.0f);
        HealthComponent->SetCurrentShield(NewShield);
    }
}

void ALA_BaseCharacter::PlayAttackMontage()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        AnimInstance->Montage_Play(AttackMontage, 1.0f);
    }
}

void ALA_BaseCharacter::UpdateTeamTag(FGameplayTag NewTeamTag)
{
    FGameplayTag ParentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team"));

    // 현재 태그들 중 "Team" 하위 태그들만 따로 추출
    FGameplayTagContainer OldTeamTags = TeamTags.Filter(FGameplayTagContainer(ParentTeamTag));

    // 찾아낸 기존 팀 태그들을 모두 제거 후 새로운 갱신 태그 추가
    TeamTags.RemoveTags(OldTeamTags);
    TeamTags.AddTag(NewTeamTag);
}

// =============================================================================
// 🎯 9. [이벤트 릴레이] 컴포넌트에서 변한 값을 받아 기존 블루프린트 UI 딜리게이트로 뿜어내 주는 가교 함수
// =============================================================================
void ALA_BaseCharacter::HandleHealthChanged(float CurrentHP, float MaxHP)
{
    if (OnHealthChanged.IsBound())
    {
        OnHealthChanged.Broadcast(CurrentHP);
    }
}

void ALA_BaseCharacter::HandleShieldChanged(float NewCurrentShield, float NewMaxShield)
{
    if (OnShieldChanged.IsBound())
    {
        OnShieldChanged.Broadcast(NewCurrentShield);
    }
}
