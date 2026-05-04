// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_AllyAIController.h"
#include "LA_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ALA_AllyAIController::ALA_AllyAIController():
	SightConfig(nullptr)
{
	UAIPerceptionComponent* NewPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*NewPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 65.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	//SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	NewPerception->ConfigureSense(*SightConfig);
	NewPerception->SetDominantSense(SightConfig->GetSenseImplementation());

}

void ALA_AllyAIController::OntargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

	// 감지
	if (Stimulus.WasSuccessfullySensed())
	{
		if (ALA_BaseCharacter* DetectedCharacter = Cast<ALA_BaseCharacter>(Actor))
		{
			if (Actor->ActorHasTag(FName("Player")) || Actor->ActorHasTag(FName("Ally")))
			{
				return;
			}
			
			// 적 감지했을 때
			// 적이 사정거리 바깥에 있으면 이동
			
            GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), DetectedCharacter);
		}
	}
}


void ALA_AllyAIController::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this, &ALA_AllyAIController::OntargetDetected
	);
}
