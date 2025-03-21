// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Weapon/ValhallaWeaponAxe.h"

#include "Character/ValhallaPlayerCharacter.h"

#include "Net/UnrealNetwork.h"

AValhallaWeaponAxe::AValhallaWeaponAxe()
{
	/*
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableRef(TEXT("/Script/Engine.DataTable'/Game/_Valhalla/Data/Weapon/DT_AxeStatus.DT_AxeStatus'"));
	if (DataTableRef.Object)
	{
		WeaponStatusData = DataTableRef.Object;
	}
	*/

	CurrentComboCount = 0;
	MaxComboCount = 1;
}

UAnimMontage* AValhallaWeaponAxe::GetComboAnim(const uint8 InCurrentComboCount)
{
	if (CurrentComboCount >= ComboAnims.Num())
	{
		return nullptr;
	}

	return ComboAnims[InCurrentComboCount];
}

void AValhallaWeaponAxe::PostInitProperties()
{
	Super::PostInitProperties();


}

void AValhallaWeaponAxe::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaWeaponAxe, CurrentComboCount);
}

void AValhallaWeaponAxe::OnRep_CurrentComboCount()
{
	/*
	AValhallaPlayerCharacter* OwningCharacter = GetOwner<AValhallaPlayerCharacter>();
	if (OwningCharacter)
	{
		float MontagePlaySpeed = OwningCharacter->GetFinalAttackSpeed();

		if (CurrentComboCount == 0)
		{
			OwningCharacter->GetMesh()->GetAnimInstance()->Montage_Play(ComboAnims[0], MontagePlaySpeed);
		}
		else
		{
			OwningCharacter->GetMesh()->GetAnimInstance()->Montage_Play(ComboAnims[1], MontagePlaySpeed);
		}
	}
	*/
	//ComboAttack(this);
}

void AValhallaWeaponAxe::ComboAttack(AValhallaWeaponBase* InWeaponBase)
{
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(InWeaponBase->GetOwner()))
	{
		float MontagePlaySpeed = CombatInterface->GetCharacterAttackSpeed();

		if (CurrentComboCount == 0)
		{
			GetOwner<ACharacter>()->GetMesh()->GetAnimInstance()->Montage_Play(ComboAnims[0], MontagePlaySpeed);
		}
		else
		{
			GetOwner<ACharacter>()->GetMesh()->GetAnimInstance()->Montage_Play(ComboAnims[1], MontagePlaySpeed);
		}
	}
}
