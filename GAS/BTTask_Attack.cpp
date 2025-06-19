#include "BTTask_Attack.h"
#include "AIControllerEnemy.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack Target";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    ACharacter* AICharacter = Cast<ACharacter>(AIController->GetPawn());
    if (!AICharacter) return EBTNodeResult::Failed;

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (!BlackboardComp) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("ObjectKey")));
    if (!Target) return EBTNodeResult::Failed;

    UE_LOG(LogTemp, Warning, TEXT("¡La IA está atacando a %s!"), *Target->GetName());
	return EBTNodeResult::Type();
}
