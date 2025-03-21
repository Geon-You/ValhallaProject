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
	// ���������� �����ϵ���
	if (HasAuthority())
	{
		AActor* OwningActor = GetOwner();
		check(OwningActor);

		ICombatInterface* CombatInterface = Cast<ICombatInterface>(OwningActor);

		if (OtherActor == OwningActor)	// ���� �����ڿ��� �ݸ����� �������� �ʵ���
		{
			return;
		}

		if (HitActors.Contains(OtherActor))	// �̹� �ǰ� ���ؼ� �迭�� ����� ���Ͷ�� �� �������� ������ �ʿ䰡 ������ ����
		{
			return;
		}

		// �������� ��
		if (OtherActor && CombatInterface)
		{
			HitActors.AddUnique(OtherActor);	// ó�� ������ ���Ͷ��(�ߺ� ������ �ƴ϶��) HitActors �迭�� ����

			/*
			// ���� �޶�߸� �������� �ֵ���
			if (IsSameTeam(OwningActor, OtherActor))
			{
				float Damage = CombatInterface->GetCharacterAttackPower() + WeaponPhysicalAttack;
				AController* DamageInstigator = GetOwner<APawn>()->GetController();

				UGameplayStatics::ApplyDamage(OtherActor, Damage, DamageInstigator, this, nullptr);
			}
			*/
			// �� �˻� ������ ĳ������ TakeDamage �Լ����� �ϰ� �־ �������� �˻��ϱ� �Ⱦ �׳� ���⼱ ������
			
			// ���� �����͵��� �����ͼ� ����� ĳ������ ���ݷ��� �ջ��� ����� �������� ����
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
	// ĳ������ ���Ȱ� ���������� ������ �´� �� ������ ��ġ���� ���ͼ� ���� ����
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
	// ������ ���� ������(1Ÿ�� ���� ������) �ִԳ�Ƽ���̸� ���� �迭 �ʱ�ȭ
	HitActors.Empty();
}

