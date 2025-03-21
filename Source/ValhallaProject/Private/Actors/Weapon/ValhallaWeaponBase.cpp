// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Weapon/ValhallaWeaponBase.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

#include "Others/Structs.h"
#include "Interface/CombatInterface.h"
#include "Interface/TeamInterface.h"

#include "Others/Debug.h"

AValhallaWeaponBase::AValhallaWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(WeaponMesh);
	WeaponCollisionBox->SetBoxExtent(FVector(20.f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxBeginOverlap);
	WeaponCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxEndOverlap);

	// Weapon Status Section
	WeaponLevel = 1;
	WeaponMaxLevel = 3;
	WeaponType = EPlayerWeaponType::Axe;
}

void AValhallaWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AValhallaWeaponBase::PostInitProperties()
{
	Super::PostInitProperties();

}

void AValhallaWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaWeaponBase, WeaponPhysicalAttack);
	DOREPLIFETIME(AValhallaWeaponBase, WeaponMagicalAttack);
	DOREPLIFETIME(AValhallaWeaponBase, AdditionalPhysicalDefence);
	DOREPLIFETIME(AValhallaWeaponBase, AdditionalMagicalDefence);
	DOREPLIFETIME(AValhallaWeaponBase, WeaponAttackSpeed);
	DOREPLIFETIME(AValhallaWeaponBase, WeaponLevel);
	DOREPLIFETIME(AValhallaWeaponBase, WeaponMaxLevel);
}

void AValhallaWeaponBase::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 동작하도록
	if (HasAuthority())
	{
		AActor* OwningActor = GetOwner();
		check(OwningActor);

		ICombatInterface* CombatInterface = Cast<ICombatInterface>(OwningActor);

		if (OtherActor == OwningActor)	// 무기 소유자에겐 콜리전이 반응하지 않도록
		{
			return;
		}

		if (HitActors.Contains(OtherActor))	// 이미 피격 당해서 배열에 저장된 액터라면 또 데미지를 적용할 필요가 없으니 리턴
		{
			return;
		}

		// 공격했을 때
		if (OtherActor && CombatInterface)
		{
			HitActors.AddUnique(OtherActor);	// 처음 공격한 액터라면(중복 공격이 아니라면) HitActors 배열에 저장

			/*
			// 팀이 달라야만 데미지를 주도록
			if (IsSameTeam(OwningActor, OtherActor))
			{
				float Damage = CombatInterface->GetCharacterAttackPower() + WeaponPhysicalAttack;
				AController* DamageInstigator = GetOwner<APawn>()->GetController();

				UGameplayStatics::ApplyDamage(OtherActor, Damage, DamageInstigator, this, nullptr);
			}
			*/
			// 팀 검사 로직을 캐릭터의 TakeDamage 함수에서 하고 있어서 이중으로 검사하기 싫어서 그냥 여기선 빼버림
			
			// 스탯 데이터들을 가져와서 무기와 캐릭터의 공격력을 합산한 결과로 데미지를 가함
			float Damage = CombatInterface->GetCharacterAttackPower() + WeaponPhysicalAttack;
			AController* DamageInstigator = GetOwner<APawn>()->GetController();

			UGameplayStatics::ApplyDamage(OtherActor, Damage, DamageInstigator, this, nullptr);
		}
	}
}

void AValhallaWeaponBase::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

bool AValhallaWeaponBase::IsSameTeam(AActor* InOwnerActor, AActor* InOtherActor)
{
	ITeamInterface* OwnerTeamInterface = Cast<ITeamInterface>(InOwnerActor);
	ITeamInterface* OtherTeamInterface = Cast<ITeamInterface>(InOtherActor);

	check(OwnerTeamInterface && OtherTeamInterface);

	return OwnerTeamInterface->GetActorTeamType() != OtherTeamInterface->GetActorTeamType();
}

void AValhallaWeaponBase::Initialize()
{	
	ensure(WeaponStatusData);

	SetWeaponStatus(WeaponLevel);
}

void AValhallaWeaponBase::SetWeaponStatus(const uint8 InWeaponLevel)
{
	// 캐릭터의 스탯과 마찬가지로 레벨에 맞는 행 데이터 수치들을 블러와서 스탯 적용
	FWeaponStatusDataTable* FoundedRow = WeaponStatusData->FindRow<FWeaponStatusDataTable>(FName(FString::FromInt(InWeaponLevel)), TEXT("Cannot find DataTable"));
	if (FoundedRow)
	{
		WeaponPhysicalAttack = FoundedRow->PhysicalAttack;
		WeaponMagicalAttack = FoundedRow->MagicalAttack;
		AdditionalPhysicalDefence = FoundedRow->PhysicalDefense;
		AdditionalMagicalDefence = FoundedRow->MagicalDefense;
		WeaponAttackSpeed = FoundedRow->AttackSpeed;
	}
}

void AValhallaWeaponBase::WeaponUpgrade()
{
	if (WeaponLevel <= WeaponMaxLevel - 1)
	{
		WeaponLevel += 1;

		SetWeaponStatus(WeaponLevel);
	}
}

void AValhallaWeaponBase::ClearHitActors()
{
	// 공격이 끝날 때마다(1타씩 끝날 때마다) 애님노티파이를 통해 배열 초기화
	HitActors.Empty();
}

