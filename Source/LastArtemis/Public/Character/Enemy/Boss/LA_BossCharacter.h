// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_BossCharacter.generated.h"
UENUM(BlueprintType)
enum class EBossPhase : uint8
{
    Phase1 UMETA(DisplayName = "Phase 1"),
    Phase2 UMETA(DisplayName = "Phase 2"),
    Phase3 UMETA(DisplayName = "Phase 3")
};

UCLASS()
class LASTARTEMIS_API ALA_BossCharacter : public ALA_BaseCharacter
{
    GENERATED_BODY()

public:
    ALA_BossCharacter();

    // 페이즈 값을 외부(컨트롤러 등)에서 읽을 수 있도록 Getter 추가
    FORCEINLINE EBossPhase GetCurrentPhase() const { return CurrentPhase; }

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

protected:
    virtual void BeginPlay() override;

    // 현재 페이즈 상태를 추적하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    EBossPhase CurrentPhase;

    // 페이즈 전환을 체크하는 함수
    UFUNCTION(Category = "Boss")
    void CheckPhaseTransition();

    // 피격 및 사망 애니메이션 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* HitMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathMontage;
};
