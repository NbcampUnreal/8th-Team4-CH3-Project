// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/Ally/LA_AllyAI.h"
#include "Character/Ally/LA_AllyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ALA_AllyAI::ALA_AllyAI()
{

}

void ALA_AllyAI::Attack()
{

}

void ALA_AllyAI::Heal()
{

}

void ALA_AllyAI::Decontaminate()
{

}

//float ALA_AllyAI::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
//{
//    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
//
//    ALA_BaseCharacter* Attacker = Cast<ALA_BaseCharacter>(DamageCauser);
//    if (Attacker)
//    {
//        ALA_AllyAIController* AllyController = Cast<ALA_AllyAIController>(GetController());
//        if (AllyController)
//        {
//            UBlackboardComponent* Blackboard = AllyController->GetBlackboardComponent();
//            if (Blackboard)
//            {
//                Blackboard->SetValueAsObject(FName("TargetActor"), Attacker);
//            }
//        }
//    }
//
//    return ActualDamage;
//
//}

void ALA_AllyAI::TakeDamageCustom(float DamageAmount)
{
    Super::TakeDamageCustom(DamageAmount);
}
