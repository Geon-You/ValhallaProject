// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Others/Enums.h"
#include "VehicleInterface.generated.h"

class USkeletalMeshComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVehicleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VALHALLAPROJECT_API IVehicleInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual ESeatType GetSeat() = 0;
	virtual void SetPrimarySeat(AActor* InActor) = 0;
	virtual void SetSecondarySeat(AActor* InActor) = 0;
	virtual FVector VehicleSecondarySeatGetOffLocation() = 0;
	virtual USkeletalMeshComponent* GetVehicleMesh() = 0;
};
