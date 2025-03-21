// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Construction/ValhallaConstruction.h"

#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"

#include "Widget/ValhallaHealthWidget.h"
#include "Interface/GoldInterface.h"

#include "Others/Debug.h"

AValhallaConstruction::AValhallaConstruction()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	ConstructionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConstructionMesh"));
	SetRootComponent(ConstructionMesh);

	ConstructionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConstructionWidgetComponent"));
	ConstructionWidgetComponent->SetupAttachment(RootComponent);

	MaxHealth = 500.f;
	ConstructionDefense = 10.f;

	TeamType = EPlayerTeamType::None;

	ActorType = EActorType::Construction;

	GoldGivenAmount = 150;
}

void AValhallaConstruction::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		InitializeStatus();	// ���� ü�� �ʱ�ȭ
	}

	// �����ڿ��� ������ ü�¹� UI�� ��������Ʈ�� ���ε�
	UValhallaHealthWidget* HealthWidget = Cast<UValhallaHealthWidget>(ConstructionWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		OnConstructionHealthChangedDelegate.BindUObject(HealthWidget, &UValhallaHealthWidget::SetHealthBar);
	}
}

void AValhallaConstruction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ���� ü���� ��Ʈ��ũ ����ȭ�� ���� ���ø����̼� ������ ���
	DOREPLIFETIME(AValhallaConstruction, CurrentHealth);
}

float AValhallaConstruction::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (EventInstigator == nullptr)	// ������ �����Ͱ� �� �����Ͷ�� ����
	{
		return 0.f;
	}

	if (ITeamInterface* TeamInterface = Cast<ITeamInterface>(EventInstigator->GetPawn()))
	{
		// �� ���� �������� �ƴ϶�� ������ ���� X
		if (TeamInterface->GetActorTeamType() == TeamType)
		{
			return 0.f;
		}
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float Damage = FMath::Clamp(DamageAmount - ConstructionDefense, 0, DamageAmount);	// ������ ������ ������ �����ؼ� �׳� ���� ������ - ���� = ���� ������ �� ����

	if (HasAuthority())
	{
		if (Damage > 0)
		{
			float NewCurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, CurrentHealth);
			CurrentHealth = NewCurrentHealth;

			// ������ ���� ��밡 �÷��̾��̰ų� ����������
			if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(EventInstigator->GetPawn<APawn>()))
			{
				if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
				{
					// ����� ü���� �����ڿ��� ������� �ϴϱ� ü�¹� UI�� ��ġ�� ������Ʈ
					OnConstructionHealthChanged(CurrentHealth, MaxHealth, EventInstigator);
				}
			}
		}

		if (CurrentHealth <= 0)
		{
			// �ı��Ǹ� ������ ���� ���濡�� ��带 ��
			// �������� ž�� ���¿��� �ı��ص� ��� ��ߵǴµ�...
			if (IGoldInterface* InstigatorCharacter = Cast<IGoldInterface>(EventInstigator->GetPawn()))
			{
				// ������ �̴Ͼ��� �ƴϰ� ĳ���Ͷ�� ��带 ��
				if (InstigatorCharacter->IsPlayerCharacter())
				{
					InstigatorCharacter->GainGold(GoldGivenAmount);
				}
			}

			// ���� ���� ������ �ı�
			DestroyConstruction();
		}
	}

	return Damage;
}

void AValhallaConstruction::Destroyed()
{
	Super::Destroyed();

	// ���⿡�� �ı��� ���� ȿ�� ���
}

void AValhallaConstruction::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0)
	{
		//Debug::Print(TEXT("Construction Destroyed"), 5.f);
	}
}

void AValhallaConstruction::InitializeStatus()
{
	CurrentHealth = MaxHealth;
}

void AValhallaConstruction::DestroyConstruction()
{
	Destroy();
}

EPlayerTeamType& AValhallaConstruction::GetActorTeamType()
{
	return TeamType;
}

EActorType& AValhallaConstruction::GetActorTypeTag()
{
	return ActorType;
}

void AValhallaConstruction::OnConstructionHealthChanged_Implementation(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController)
{
	// ��������Ʈ ����
	OnConstructionHealthChangedDelegate.ExecuteIfBound(InCurrentHealth, InMaxHealth, InInstigatorController);
}

