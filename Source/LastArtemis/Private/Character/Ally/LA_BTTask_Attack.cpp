// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_Attack.h"
#include "Character/LA_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

EBTNodeResult::Type ULA_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ALA_BaseCharacter* Target = Cast<ALA_BaseCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
    {

        FHitResult HitResult;
        FVector Start = OwnerComp.GetOwner()->GetActorLocation();
        FVector End = Target->GetActorLocation();

        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 2.f);

        float Distance = FVector::Dist(Start, End);
        if (Distance > AttackRange) return EBTNodeResult::Failed;

        // 라인트레이싱
        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Pawn

        );

        

        if (ALA_BaseCharacter* HitTarget = Cast<ALA_BaseCharacter>(HitResult.GetActor()))
        {
            UE_LOG(LogTemp, Warning, TEXT("Attack Task Executing!"));
            // 맞은 대상에 대미지 적용 로직
            HitTarget->TakeDamageCustom(AttackPower);
            return EBTNodeResult::Succeeded;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"),
                HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"));
        }
    }

    return EBTNodeResult::Failed;
}
