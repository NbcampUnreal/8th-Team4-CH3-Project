// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ally/LA_BTTask_Attack.h"
#include "Character/LA_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/Ally/LA_AllyAIController.h"

EBTNodeResult::Type ULA_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogTemp, Warning, TEXT("Attack Task Executing!"));

    UObject* RawTarget = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));
    UE_LOG(LogTemp, Warning, TEXT("TargetActor: %s"), RawTarget ? *RawTarget->GetName() : TEXT("None"));

    if (ALA_BaseCharacter* Target = Cast<ALA_BaseCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"))))
    {

        FHitResult HitResult;
        APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
        if (!OwnerPawn) return EBTNodeResult::Failed;
        FVector Start = OwnerPawn->GetActorForwardVector() * 50.f + OwnerPawn->GetActorLocation();
        FVector End = OwnerPawn->GetActorForwardVector() * 500.f + Start;

        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 2.f);

        float Distance = FVector::Dist(Start, End);
        //if (Distance > AttackRange) return EBTNodeResult::Failed;

        FCollisionQueryParams QueryParams(FName("Attack"), false);
        QueryParams.AddIgnoredActor(OwnerComp.GetOwner());
        QueryParams.AddIgnoredActor(OwnerPawn);

        // 라인트레이싱
        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility,
            QueryParams
        );

        DrawDebugSphere(GetWorld(), HitResult.Location, 10.f, 16, FColor::Red, false, 2.f, 0, 1.f);
        UE_LOG(LogTemp, Warning, TEXT("Debug Start"));

        if (ALA_BaseCharacter* HitTarget = Cast<ALA_BaseCharacter>(HitResult.GetActor()))
        {
            // 맞은 대상에 대미지 적용 로직
            UE_LOG(LogTemp, Warning, TEXT("Attack Succeeded"));
            
            HitTarget->TakeDamageCustom(AttackPower);
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Enemy Health : % f / % f"), HitTarget->CurrentHealth, HitTarget->MaxHealth));

            if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
            {
                if (HitTarget->bIsDead)
                {
                    Blackboard->ClearValue(GetSelectedBlackboardKey());
                }
                
            }


            return EBTNodeResult::Succeeded;

        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"),
                HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"));
        }
    }

    


    UE_LOG(LogTemp, Warning, TEXT("Failed"));
    return EBTNodeResult::Failed;
}
