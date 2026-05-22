#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Character/LA_BaseCharacter.h"

class ULA_HealthComponent;
class UWidgetComponent;

#include "LA_EnemyCharacter.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemyCharacter : public ALA_BaseCharacter, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    ALA_EnemyCharacter();

    // --- 전투 및 데미지 시스템 ---
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Die() override;

    // --- 애니메이션 노티파이 연결용 ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnemyMeleeAttackCheck();

    // --- 인터페이스 구현 ---
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    void PlayAttackMontage();

    void PlayAttackMontageWithComp(class UBehaviorTreeComponent* OwnerComp, const class UBTTaskNode* ActionNode);

    FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;

    // --- 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ULA_HealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, Category = "UI")
    TObjectPtr<class UWidgetComponent> HealthWidgetComp;

    // --- 대미지 UI (누적 방식) ---
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> DamageTextClass;

    UPROPERTY()
    TArray<class UUserWidget*> ActiveDamageWidgets;

    void ExecuteShowDamageText();

    float AccumulatedDamage = 0.0f;
    FTimerHandle DamageDisplayTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
    bool bIsAttacking = false;

    // --- 애니메이션 에셋 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* HitMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathMontage;

    // --- 데이터 (태그/전투 수치) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer GameplayTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackRadius = 40.0f;
};
