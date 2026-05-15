#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Character/LA_BaseCharacter.h"
#include "LA_EnemyCharacter.generated.h"

UCLASS()
class LASTARTEMIS_API ALA_EnemyCharacter : public ALA_BaseCharacter, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	ALA_EnemyCharacter();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Die() override;

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnemyMeleeAttackCheck();


protected:
	virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class ULA_HealthComponent* HealthComp;

    // 대미지 텍스트 출력 함수
    void ExecuteShowDamageText();

    UPROPERTY(VisibleAnywhere, Category = "UI")
    TObjectPtr<class UWidgetComponent> HealthWidgetComp;

    // 대미지 텍스트를 스폰할 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> DamageTextClass;

    // 사라지게 할 위젯 리스트
    UPROPERTY()
    TArray<UUserWidget*> ActiveDamageWidgets;

    float AccumulatedDamage = 0.0f;
    FTimerHandle DamageDisplayTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer GameplayTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackRange = 120.0f;

    // 공격 판정 반지름 (구체 트레이스)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackRadius = 40.0f;

};
