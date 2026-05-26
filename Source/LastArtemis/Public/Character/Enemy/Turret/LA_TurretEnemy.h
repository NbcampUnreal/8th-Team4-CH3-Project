// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_TurretEnemy.generated.h"

class UWidgetComponent;

UCLASS()
class LASTARTEMIS_API ALA_TurretEnemy : public ALA_BaseCharacter
{
    GENERATED_BODY()

public:
    ALA_TurretEnemy();

protected:
    virtual void BeginPlay() override;
    float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
                     AActor* DamageCauser);
    virtual void Tick(float DeltaTime) override;

    // [중요 수정] 중복되던 ULA_HealthComponent* HealthComp; 선언을 제거했습니다!
    // 이제 부모 클래스인 LA_BaseCharacter가 물려준 'HealthComponent' 변수를 그대로 공유해서 사용합니다.

    UPROPERTY(VisibleAnywhere, Category = "UI")
    TObjectPtr<UWidgetComponent> HealthWidgetComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* DeathExplosionEffect;

    // 포탑 헤드 (회전할 부분)
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TurretHead;

    // 사거리 및 회전 속도
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DetectionRange;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed;

    // 발사 관련
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class ALA_Projectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float FireRate;

    FTimerHandle FireTimerHandle;

    // 타겟 탐색 및 공격 로직
    void FindTarget();
    void FireProjectile();
    bool CheckLineOfSight(AActor* TargetActor);

    virtual void UpdateTeamTag(FGameplayTag NewTeamTag) override { SwitchTeam(NewTeamTag); }
    void SwitchTeam(FGameplayTag NewTeamTag);


    virtual void Die() override;

    void ExecuteShowDamageText();

    // 델리게이트 매칭용 함수 (형식 유지)
    UFUNCTION()
    void OnHealthChangedCallback(float CurrentHP, float MaxHP);

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> DamageTextClass;

    UPROPERTY()
    TArray<class UUserWidget*> ActiveDamageWidgets;

    float AccumulatedDamage = 0.0f;

    FTimerHandle DamageDisplayTimer;

    UPROPERTY()
    AActor* CurrentTarget;
};
