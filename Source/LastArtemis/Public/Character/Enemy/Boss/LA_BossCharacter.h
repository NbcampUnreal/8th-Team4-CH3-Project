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

    // --- 전투 및 가상 함수 오버라이드 ---
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

protected:
    virtual void BeginPlay() override;

    // 🎯 [핵심 보완] .cpp에 구현된 보스 수치 세팅 로직을 안전하게 실행하기 위한 선언 추가!
    virtual void PostInitializeComponents() override;

    // --- 보스 전용 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* BossStaticMesh;

    // 현재 페이즈 상태를 추적하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    EBossPhase CurrentPhase;

    // 페이즈 전환을 체크하는 함수
    UFUNCTION(Category = "Boss")
    void CheckPhaseTransition();

    // --- 니아가라 비주얼 이펙트 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UNiagaraSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UNiagaraSystem* DeathExplosionEffect;
};
