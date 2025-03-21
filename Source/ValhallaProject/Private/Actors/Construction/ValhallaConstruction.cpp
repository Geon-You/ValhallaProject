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
		InitializeStatus();	// 현재 체력 초기화
	}

	// 공격자에게 보여줄 체력바 UI를 델리게이트와 바인딩
	UValhallaHealthWidget* HealthWidget = Cast<UValhallaHealthWidget>(ConstructionWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		OnConstructionHealthChangedDelegate.BindUObject(HealthWidget, &UValhallaHealthWidget::SetHealthBar);
	}
}

void AValhallaConstruction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 현재 체력을 네트워크 동기화를 위해 리플리케이션 변수로 등록
	DOREPLIFETIME(AValhallaConstruction, CurrentHealth);
}

float AValhallaConstruction::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (EventInstigator == nullptr)	// 공격자 포인터가 널 포인터라면 리턴
	{
		return 0.f;
	}

	if (ITeamInterface* TeamInterface = Cast<ITeamInterface>(EventInstigator->GetPawn()))
	{
		// 적 팀의 데미지가 아니라면 데미지 적용 X
		if (TeamInterface->GetActorTeamType() == TeamType)
		{
			return 0.f;
		}
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float Damage = FMath::Clamp(DamageAmount - ConstructionDefense, 0, DamageAmount);	// 지금은 데미지 계산식이 간단해서 그냥 들어온 데미지 - 방어력 = 최종 데미지 로 적용

	if (HasAuthority())
	{
		if (Damage > 0)
		{
			float NewCurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, CurrentHealth);
			CurrentHealth = NewCurrentHealth;

			// 공격을 가한 상대가 플레이어이거나 공성병기라면
			if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(EventInstigator->GetPawn<APawn>()))
			{
				if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
				{
					// 변경된 체력을 공격자에게 보여줘야 하니까 체력바 UI의 수치를 업데이트
					OnConstructionHealthChanged(CurrentHealth, MaxHealth, EventInstigator);
				}
			}
		}

		if (CurrentHealth <= 0)
		{
			// 파괴되면 공격을 가한 상대방에게 골드를 줌
			// 공성병기 탑승 상태에서 파괴해도 골드 줘야되는데...
			if (IGoldInterface* InstigatorCharacter = Cast<IGoldInterface>(EventInstigator->GetPawn()))
			{
				// 상대방이 미니언이 아니고 캐릭터라면 골드를 줌
				if (InstigatorCharacter->IsPlayerCharacter())
				{
					InstigatorCharacter->GainGold(GoldGivenAmount);
				}
			}

			// 벽을 게임 내에서 파괴
			DestroyConstruction();
		}
	}

	return Damage;
}

void AValhallaConstruction::Destroyed()
{
	Super::Destroyed();

	// 여기에서 파괴시 폭발 효과 재생
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
	// 델리게이트 실행
	OnConstructionHealthChangedDelegate.ExecuteIfBound(InCurrentHealth, InMaxHealth, InInstigatorController);
}

