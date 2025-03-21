// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Weapon/ValhallaWeaponBow.h"

AValhallaWeaponBow::AValhallaWeaponBow()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableRef(TEXT("/Script/Engine.DataTable'/Game/_Valhalla/Data/Weapon/DT_BowStatus.DT_BowStatus'"));
	if (DataTableRef.Object)
	{
		WeaponStatusData = DataTableRef.Object;
	}
}

void AValhallaWeaponBow::PostInitProperties()
{
	Super::PostInitProperties();

	Initialize();
}
