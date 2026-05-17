#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameplayTagAssetInterface.h"

ALA_EnemyController::ALA_EnemyController()
{
    // AIPerception 컴포넌트 초기화
    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {
        // 헤더에서 설정한 변수값 적용 (블루프린트 수정 가능)
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 2000.0f;
        SightConfig->PeripheralVisionAngleDegrees = 60.0f;

        // 감지 대상 설정 (아군/적군/중립 모두)
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
        EnemyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // 인식 업데이트 이벤트 연결
    EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALA_EnemyController::OnTargetDetected);

    PrimaryActorTick.bCanEverTick = false;
}

void ALA_EnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BTAsset)
    {
        // Behavior Tree 실행
        if (RunBehaviorTree(BTAsset))
        {
            // 초기 위치 저장 (순찰 복귀용)
            GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
        }
    }
}

void ALA_EnemyController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // 1. GameplayTag 인터페이스 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
    if (TagInterface)
    {
        FGameplayTagContainer TargetTags;
        TagInterface->GetOwnedGameplayTags(TargetTags);

        // 2. Team.Ally (플레이어 팀) 태그 확인
        FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Team.Ally"));

        if (TargetTags.HasTag(AllyTag))
        {
            UBlackboardComponent* BB = GetBlackboardComponent();
            if (!BB) return;

            if (Stimulus.WasSuccessfullySensed())
            {
                // 플레이어 감지 성공: 블랙보드에 타겟 설정
                BB->SetValueAsObject(TEXT("TargetActor"), Actor);
                UE_LOG(LogTemp, Log, TEXT("[AI] Target Found: %s"), *Actor->GetName());
            }
            else
            {
                // 플레이어를 놓침: 블랙보드 타겟 제거 -> BT가 자동으로 순찰/대기 상태로 전환
                BB->ClearValue(TEXT("TargetActor"));
                UE_LOG(LogTemp, Log, TEXT("[AI] Target Lost"));
            }
        }
    }
}
