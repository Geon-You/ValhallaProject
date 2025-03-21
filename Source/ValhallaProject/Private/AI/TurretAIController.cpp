// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TurretAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"

#include "Actors/Turret/ValhallaTurret.h"

#include "Others/Debug.h"

ATurretAIController::ATurretAIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ATurretAIController::OnActorPerceptionUpdated);

	TargetActors.Empty();
	CurrentTargetActor = nullptr;
}

void ATurretAIController::BeginPlay()
{
	Super::BeginPlay();

}

void ATurretAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 현재 컨트롤하고 있는 폰 설정
	ControlledTurret = Cast<AValhallaTurret>(InPawn);

	// 팀 설정
	SetGenericTeamId(FGenericTeamId((uint8)ControlledTurret->GetTeamType()));

	// 비헤이비어 트리 실행
	if (BehaviorTree)
	{
		BehaviorTree->BlackboardAsset = BlackboardData;
		RunBehaviorTree(BehaviorTree);
	}
}

void ATurretAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurretAIController, TargetActors);
	DOREPLIFETIME(ATurretAIController, CurrentTargetActor);
}

void ATurretAIController::OnActorPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 시야에 의한 업데이트는 무언가가 감지망 내로 들어오거나 감지망 밖으로 나갈 때 호출됨.
	// 호출됐을 경우 업데이트 된 액터가 감지되지 않은 상태라면 배열에 추가 (감지망 안으로 들어온 케이스), 이미 감지된 상태라면 배열에서 삭제 (감지망 밖으로 나간 케이스)
	if (UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus) == UAISense_Sight::StaticClass())
	{
		ITeamInterface* TurretTeamInterface = Cast<ITeamInterface>(ControlledTurret);
		ITeamInterface* TargetTeamInterface = Cast<ITeamInterface>(Actor);

		// 감지망 내로 들어온 타겟이 같은 팀이라면 리턴
		if (TurretTeamInterface->GetActorTeamType() == TargetTeamInterface->GetActorTeamType())
		{
			return;
		}

		if (TargetActors.Contains(Actor))
		{
			// 감지망 밖으로 나간 경우
			TargetActors.RemoveSingle(Actor);
		}
		else
		{
			// 감지망 안으로 들어온 경우
			TargetActors.AddUnique(Actor);
		}

		// 업데이트 될 때마다 폰(터렛)에서 가장 가까운 액터를 배열에서 찾아 반환
		// 반환받은 액터(감지된)를 비헤이비어 트리의 타겟 액터로 설정
		float DetectedDistance = 0.f;
		CurrentTargetActor = UGameplayStatics::FindNearestActor(ControlledTurret->GetActorLocation(), TargetActors, DetectedDistance);

		// 블랙보드의 현재 타겟 값으로 설정
		GetBlackboardComponent()->SetValueAsObject(TEXT("CurrentTarget"), CurrentTargetActor);
	}
}
