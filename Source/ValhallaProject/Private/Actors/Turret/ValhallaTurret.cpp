// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Turret/ValhallaTurret.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "AI/TurretAIController.h"
#include "Actors/Turret/ValhallaTurretBullet.h"
#include "Widget/ValhallaHealthWidget.h"
#include "Interface/GoldInterface.h"

#include "Others/Debug.h"

AValhallaTurret::AValhallaTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	SetRootComponent(TurretMesh);

	BulletSpawnTransform = CreateDefaultSubobject<USceneComponent>(TEXT("BulletSpawnTransform"));
	BulletSpawnTransform->SetupAttachment(RootComponent);

	TurretWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConstructionWidgetComponent"));
	TurretWidgetComponent->SetupAttachment(RootComponent);

	TeamType = EPlayerTeamType::None;

	ActorType = EActorType::Turret;

	MaxHealth = 300.f;
	TurretDefense = 15.f;

	GoldGivenAmount = 150;
}

void AValhallaTurret::BeginPlay()
{
	Super::BeginPlay();
	
	check(TurretBulletClass);

	if (HasAuthority())
	{
		InitializeStatus();	// 건물과 기능이 동일
	}

	// 건물과 동일
	UValhallaHealthWidget* HealthWidget = Cast<UValhallaHealthWidget>(TurretWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		OnTurretHealthChangedDelegate.BindUObject(HealthWidget, &UValhallaHealthWidget::SetHealthBar);
	}
}

void AValhallaTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AValhallaTurret::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AValhallaTurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 터렛 컨트롤러를 지금 현재의 컨트롤러(빙의된 터렛 AI 컨트롤러)로 저장
	// 추후에 사용할 일이 있을 수도 있어서
	if (ATurretAIController* TurretAIController = Cast<ATurretAIController>(NewController))
	{
		TurretController = TurretAIController;
	}
}

void AValhallaTurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 마찬가지로 네트워크 동기화를 위해 리플리케이션 변수로 등록
	// 이 매크로가 C++에서 멤버변수를 리플리케이션 시스템에 등록하기 위한 매크로
	// "Net/UnrealNetwork.h" 를 인클루드 해야지 사용 가능
	DOREPLIFETIME(AValhallaTurret, CurrentHealth);
}

float AValhallaTurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 전반적으로 건물 쪽 로직과 동일함
	if (EventInstigator == nullptr)
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

	float Damage = FMath::Clamp(DamageAmount - TurretDefense, 0, DamageAmount);

	if (HasAuthority())
	{
		if (Damage > 0)
		{
			float NewCurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, CurrentHealth);
			CurrentHealth = NewCurrentHealth;

			// 공격을 가한 상대가 플레이어라면
			if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(EventInstigator->GetPawn<APawn>()))
			{
				if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
				{
					OnTurretHealthChanged(CurrentHealth, MaxHealth, EventInstigator);
				}
			}
		}

		if (CurrentHealth <= 0)
		{
			// 파괴되면 공격을 가한 상대방에게 골드를 줌
			if (IGoldInterface* InstigatorCharacter = Cast<IGoldInterface>(EventInstigator->GetPawn()))
			{
				// 상대방이 미니언이 아니고 캐릭터라면 골드를 줌
				if (InstigatorCharacter->IsPlayerCharacter())
				{
					InstigatorCharacter->GainGold(GoldGivenAmount);
				}
			}

			DestroyTurret();
		}
	}

	return Damage;
}

void AValhallaTurret::Destroyed()
{
	Super::Destroyed();

	// 여기에서 파괴시 폭발 효과 재생
}

void AValhallaTurret::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 사용 X
}

void AValhallaTurret::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0)
	{
		//Debug::Print(TEXT("Turret Destoryed"), 5.f);
	}
}

void AValhallaTurret::InitializeStatus()
{
	CurrentHealth = MaxHealth;
}

void AValhallaTurret::DestroyTurret()
{
	Destroy();
}

EPlayerTeamType& AValhallaTurret::GetActorTeamType()
{
	return TeamType;
}

EActorType& AValhallaTurret::GetActorTypeTag()
{
	return ActorType;
}

void AValhallaTurret::OnTurretHealthChanged_Implementation(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController)
{
	OnTurretHealthChangedDelegate.ExecuteIfBound(InCurrentHealth, InMaxHealth, InInstigatorController);
}

void AValhallaTurret::BulletFire_Implementation(AActor* InTargetActor)
{
	if (InTargetActor == nullptr)
		return;

	// 총구에서 불 생성
	FVector SpawnLocation = BulletSpawnTransform->GetComponentLocation();
	FVector SpawnRelativeLocation = BulletSpawnTransform->GetRelativeLocation();

	FRotator SpawnRotator = BulletSpawnTransform->GetComponentRotation();
	FTransform SpawnTransform(SpawnRotator, SpawnLocation, FVector(1.f, 1.f, 1.f));

	// 불 발사체 스폰
	// SpawnActorDeferred 함수를 사용해서 생성하면 생성되기 전에 생성할 객체의 변수들을 미리 설정할 수 있음
	AValhallaTurretBullet* Bullet = GetWorld()->SpawnActorDeferred<AValhallaTurretBullet>(TurretBulletClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (Bullet)
	{
		Bullet->SetTeamType(TeamType);	// 팀 정하고 (상대방에게만 데미지를 주기 위해)
		Bullet->SetTargetActor(InTargetActor);	// 목표물을 정하고 (목표물을 향해 일직선으로 날라감)
		Bullet->SetHitEffectScale(FVector(5.f, 5.f, 5.f));	// 불이 뭔가에 맞아서 터질 때 이펙트 크기를 정하고

		FVector TargetLocation = InTargetActor->GetActorLocation() + FVector(0.f, 0.f, -25.f);	// 타겟의 위치를 지정해준 다음에
		FRotator BulletRot = UKismetMathLibrary::FindLookAtRotation(SpawnTransform.GetLocation(), TargetLocation);	// 타겟의 위치를 향하는 방향을 찾음
		Bullet->SetActorRotation(BulletRot);	// 그 방향을 적용해서 타겟에게 일직선으로 날라가도록

		Bullet->FinishSpawning(SpawnTransform);	// FinishSpawning 함수를 호출해야지만 생성이 완료됨. SpawnActorDeferred로 객체를 생성할 때 마지막에 이 함수를 호출하지 않으면 생성되지 않음. 주의!!!
	}
}

