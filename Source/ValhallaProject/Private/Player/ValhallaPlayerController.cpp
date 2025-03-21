// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ValhallaPlayerController.h"

#include "Net/UnrealNetwork.h"

AValhallaPlayerController::AValhallaPlayerController()
{
	ControlledPawn = nullptr;
}

void AValhallaPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaPlayerController, ControlledPawn);
}
