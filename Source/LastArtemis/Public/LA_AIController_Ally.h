#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "LA_AIController_Ally.generated.h"



UCLASS()
class LASTARTEMIS_API ALA_AIController_Ally : public AAIController
{
	GENERATED_BODY()
	
public:
	
	ALA_AIController_Ally();

	UPROPERTY(VisibleAnywhere)
	UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OntargetDetected(AActor* Actor, FAIStimulus Stimulus);

private:
	virtual void BeginPlay() override;
};
