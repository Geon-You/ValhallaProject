// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Weapon/ValhallaWeaponBase.h"
#include "ValhallaWeaponBow.generated.h"

/**
 * 
 */
UCLASS()
class VALHALLAPROJECT_API AValhallaWeaponBow : public AValhallaWeaponBase
{
	GENERATED_BODY()
	
public:
	AValhallaWeaponBow();

protected:
	virtual void PostInitProperties() override;
};
