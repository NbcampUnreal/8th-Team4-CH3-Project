// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LA_EnemyHealthWidget.h"

void ULA_EnemyHealthWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 데미지 바(DelayedHealth)가 실제 체력으로 서서히 줄어드는 연산
    if (MaxHealth > 0.0f && DelayedHealth > CurrentHealth)
    {
        DelayedHealth = FMath::FInterpTo(DelayedHealth, CurrentHealth, InDeltaTime, InterpSpeed);
        
        // 수치가 어느 정도 가까워지면 즉시 일치시켜 잔상이 남지 않게 함
        if (DelayedHealth - CurrentHealth < 0.5f)
        {
            DelayedHealth = CurrentHealth;
        }

        UpdateRatios();
    }
}

void ULA_EnemyHealthWidget::UpdateHealthBar(float Current, float Max)
{
    if (Max <= 0.0f) return;

    // 초기화 (처음 호출될 때)
    if (MaxHealth <= 0.0f)
    {
        DelayedHealth = Current;
    }

    CurrentHealth = Current;
    MaxHealth = Max;

    // 회복했을 때는 데미지 바도 즉시 따라가게 함
    if (CurrentHealth > DelayedHealth)
    {
        DelayedHealth = CurrentHealth;
    }

    UpdateRatios();
}

void ULA_EnemyHealthWidget::UpdateRatios()
{
    if (MaxHealth <= 0.0f) return;

    // 1. 실제 체력 비율 (빨간색)
    float HealthP = CurrentHealth / MaxHealth;
    
    // 2. 데미지 표시 바 비율 (흰색/노란색)
    // 현재 체력과 이전 체력(Delayed) 사이의 차이만큼 표시
    float DamageP = (DelayedHealth - CurrentHealth) / MaxHealth;
    
    // 3. 남은 빈 공간 비율
    float SpaceP = (MaxHealth - DelayedHealth) / MaxHealth;

    // 블루프린트로 세 가지 수치 전달
    NativeUpdateHealthBar(HealthP, DamageP, SpaceP);
}
