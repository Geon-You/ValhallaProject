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

	// ���� ��Ʈ���ϰ� �ִ� �� ����
	ControlledTurret = Cast<AValhallaTurret>(InPawn);

	// �� ����
	SetGenericTeamId(FGenericTeamId((uint8)ControlledTurret->GetTeamType()));

	// �����̺�� Ʈ�� ����
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
	// �þ߿� ���� ������Ʈ�� ���𰡰� ������ ���� �����ų� ������ ������ ���� �� ȣ���.
	// ȣ����� ��� ������Ʈ �� ���Ͱ� �������� ���� ���¶�� �迭�� �߰� (������ ������ ���� ���̽�), �̹� ������ ���¶�� �迭���� ���� (������ ������ ���� ���̽�)
	if (UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus) == UAISense_Sight::StaticClass())
	{
		ITeamInterface* TurretTeamInterface = Cast<ITeamInterface>(ControlledTurret);
		ITeamInterface* TargetTeamInterface = Cast<ITeamInterface>(Actor);

		// ������ ���� ���� Ÿ���� ���� ���̶�� ����
		if (TurretTeamInterface->GetActorTeamType() == TargetTeamInterface->GetActorTeamType())
		{
			return;
		}

		if (TargetActors.Contains(Actor))
		{
			// ������ ������ ���� ���
			TargetActors.RemoveSingle(Actor);
		}
		else
		{
			// ������ ������ ���� ���
			TargetActors.AddUnique(Actor);
		}

		// ������Ʈ �� ������ ��(�ͷ�)���� ���� ����� ���͸� �迭���� ã�� ��ȯ
		// ��ȯ���� ����(������)�� �����̺�� Ʈ���� Ÿ�� ���ͷ� ����
		float DetectedDistance = 0.f;
		CurrentTargetActor = UGameplayStatics::FindNearestActor(ControlledTurret->GetActorLocation(), TargetActors, DetectedDistance);

		// �������� ���� Ÿ�� ������ ����
		GetBlackboardComponent()->SetValueAsObject(TEXT("CurrentTarget"), CurrentTargetActor);
	}
}
