#include "Character/Enemy/EnemyAI/LA_EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameplayTagAssetInterface.h"


void ALA_EnemyController::BeginPlay()
{
    Super::BeginPlay();

    // 퍼셉션 컴포넌트가 존재한다면 업데이트 이벤트 연결
    if (GetAIPerceptionComponent())
    {
        GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ALA_EnemyController::OnTargetDetected);
    }
}

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
        SightConfig->PeripheralVisionAngleDegrees = 45.0f;
        SightConfig->SetMaxAge(5.0f);

        // 감지 대상 설정 (아군/적군/중립 모두)
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        SightConfig->PointOfViewBackwardOffset = 0.0f;
        SightConfig->NearClippingRadius = 0.0f;

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

    if (Actor->ActorHasTag(FName("Team.Ally")) || Actor->IsA(APawn::StaticClass()))
    {
        UBlackboardComponent* BB = GetBlackboardComponent();
        if (!BB) return;

        // 시야에 들어왔는지 확인
        if (Stimulus.WasSuccessfullySensed())
        {
            // 블랙보드에 직접 때려박기
            BB->SetValueAsObject(TEXT("TargetActor"), Actor);
            UE_LOG(LogTemp, Warning, TEXT("!!! [성공] 플레이어 포착 완료 !!!"));
        }
        else
        {
            BB->ClearValue(TEXT("TargetActor"));
            UE_LOG(LogTemp, Warning, TEXT("!!! [알림] 플레이어 놓침 !!!"));
        }
    }
}
