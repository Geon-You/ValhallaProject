// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ValhallaCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "AISystem.h"

#include "Actors/Weapon/ValhallaWeaponBase.h"

#include "Others/Debug.h"

AValhallaCharacterBase::AValhallaCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Team Section
	TeamType = EPlayerTeamType::None;

	// Player Death State Section
	PlayerCurrentState = EPlayerCurrentState::Alive;

	// AI Section
	AIPerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AIPerceptionStimuliSourceComponent"));
}

void AValhallaCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 콜리전의 오브젝트 타입을 팀 타입에 맞게 설정
	if (TeamType == EPlayerTeamType::Red)
	{
		// 팀 타입이 레드팀이면 콜리전 오브젝트 설정을 레드팀으로
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	}
	else if (TeamType == EPlayerTeamType::Blue)
	{
		// 팀 타입이 블루팀이면 콜리전 오브젝트 설정을 블루팀으로
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	}
}

void AValhallaCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AValhallaCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AValhallaCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버에서 초기화
}

void AValhallaCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 초기화
}

float AValhallaCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return DamageAmount;
}

void AValhallaCharacterBase::OnRep_PlayerCurrentState()
{
	// 죽었을 때 모든 클라이언트에서 죽는 애니메이션 재생 (동기화)
	if (PlayerCurrentState == EPlayerCurrentState::Dead)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DeadMontage);
	}
}

void AValhallaCharacterBase::OnRep_TeamType()
{
	// 모든 클라이언트에서도 콜리전의 오브젝트 타입을 팀 타입에 맞게 설정
	if (TeamType == EPlayerTeamType::Red)
	{
		// 팀 타입이 레드팀이면 콜리전 오브젝트 설정을 레드팀으로
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	}
	else if (TeamType == EPlayerTeamType::Blue)
	{
		// 팀 타입이 블루팀이면 콜리전 오브젝트 설정을 블루팀으로
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	}
}

void AValhallaCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 리플리케이션 변수들을 등록
	DOREPLIFETIME(AValhallaCharacterBase, CurrentHealth);

	DOREPLIFETIME(AValhallaCharacterBase, TeamType);

	DOREPLIFETIME(AValhallaCharacterBase, PlayerCurrentState);
	DOREPLIFETIME(AValhallaCharacterBase, GoldGivenAmount);
}

void AValhallaCharacterBase::OnRep_CurrentHealth()
{
	// 사용 X

	/*
	if (CurrentHealth <= 0.f)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DeadMontage);
	}
	else if (CurrentHealth < 100.f && CurrentHealth > 0.f)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitReactionMontage);
	}
	*/
}

void AValhallaCharacterBase::InitializeHealthAndMana()
{
	// 자식 클래스에서 override
}

void AValhallaCharacterBase::ResetHealthAndMana()
{
	// 자식 클래스에서 override
}

void AValhallaCharacterBase::Die()
{
	// 자식 클래스에서 override
}

bool AValhallaCharacterBase::IsDead(const float InCurrentHealth)
{
	// 캐릭터의 현재 체력으로 죽었는지 판단하는 함수
	if (InCurrentHealth <= 0)
	{
		return true;
	}

	return false;
}

void AValhallaCharacterBase::SetCollisionOnOff_Implementation(const bool IsCollisionOn)
{
	// 캐릭터가 죽거나 리스폰 됐을 때 콜리전 반응을 껐다가 켰다가 하는 함수
	if (!IsCollisionOn)
	{
		// ECC_GameTraceChannel2 가 Weapon, ECC_GameTraceChannel3 가 Red, ECC_GameTraceChannel4 가 블루

		// 죽었을 때
		// 콜리전 끄기
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECollisionResponse::ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

		// 포탑 감지 해제
		AIPerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
	else
	{
		// 다시 리스폰 됐을 때
		// 콜리전 켜기
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);

		// 다시 포탑에 감지될 수 있도록
		AIPerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

void AValhallaCharacterBase::ToggleWeaponCollision(bool IsCollisionEnable)
{
	// 자식 클래스에서 override
}

EPlayerTeamType& AValhallaCharacterBase::GetActorTeamType()
{
	return TeamType;
}

EActorType& AValhallaCharacterBase::GetActorTypeTag()
{
	return ActorType;
}

