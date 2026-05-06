// Fill out your copyright notice in the Description page of Project Settings.



#include "Character/Ally/LA_AllyAIController.h"
#include "Character/LA_BaseCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"


ALA_AllyAIController::ALA_AllyAIController()
{
    UAIPerceptionComponent* NewPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*NewPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1000.f;
    SightConfig->PeripheralVisionAngleDegrees = 65.f;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    NewPerception->ConfigureSense(*SightConfig);
    NewPerception->SetDominantSense(SightConfig->GetSenseImplementation());

  
}

void ALA_AllyAIController::BeginPlay()
{
	Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AllyAIController BeginPlay!"));

    if (BehaviorTreeAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("BehaviorTree Running!"));
        RunBehaviorTree(BehaviorTreeAsset);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BehaviorTree Asset is NULL!"));
    }

    AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    GetBlackboardComponent()->SetValueAsObject(FName("PlayerActor"), Player);

    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(
        this, &ALA_AllyAIController::OnTargetDetected
    );
}

void ALA_AllyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        if (ALA_BaseCharacter* DetectedCharacter = Cast<ALA_BaseCharacter>(Actor))
        {
            if (DetectedCharacter->CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally")))) {
                return;
            }
            GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), DetectedCharacter);
        }
    }
}



