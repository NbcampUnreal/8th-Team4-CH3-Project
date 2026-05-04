// Fill out your copyright notice in the Description page of Project Settings.


#include "LA_AIController_Ally.h"
#include "LA_BaseCharacter.h"

ALA_AIController_Ally::ALA_AIController_Ally():
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

void ALA_AIController_Ally::OntargetDetected(AActor* Actor, FAIStimulus Stimulus)
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
			//

		}
	}
}


void ALA_AIController_Ally::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this, &ALA_AIController_Ally::OntargetDetected
	);
}
