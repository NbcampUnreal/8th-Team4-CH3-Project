// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Ally/LA_AllyAIController.h"
#include "Character/LA_BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Component/LA_HealthComponent.h"

ALA_AllyAIController::ALA_AllyAIController()
{
    UAIPerceptionComponent* NewPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*NewPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 4000.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
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
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(FName("PlayerActor"), Player);
    }

    if (GetPerceptionComponent())
    {
        GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(
            this, &ALA_AllyAIController::OnTargetDetected
        );
    }
}

void ALA_AllyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        if (ALA_BaseCharacter* DetectedCharacter = Cast<ALA_BaseCharacter>(Actor))
        {
            // 같은 아군 진영(`Team.Ally`) 태그를 가졌다면 시야 타겟 지정에서 제외 (오사 차단)
            if (DetectedCharacter->CharacterTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Team.Ally"))))
            {
                return;
            }

            UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
            if (BlackboardComp)
            {
                ALA_BaseCharacter* CurrentTarget = Cast<ALA_BaseCharacter>(BlackboardComp->GetValueAsObject(FName("TargetActor")));

                // 🎯 [심볼 에러 완벽 수정]
                // 기존 타겟이 이미 존재하고, 그 타겟이 아직 '살아있는 상태(IsDead == false)'라면 타겟을 바꾸지 않고 기존 공격을 유지합니다!
                if (CurrentTarget && CurrentTarget->GetHealthComponent() && !CurrentTarget->GetHealthComponent()->IsDead())
                {
                    return;
                }

                // 기존 타겟이 없거나 이미 죽은 타겟이라면 새로 감지된 살아있는 적을 블랙보드에 조준 록온합니다!
                BlackboardComp->SetValueAsObject(FName("TargetActor"), DetectedCharacter);
            }
        }
    }
}
