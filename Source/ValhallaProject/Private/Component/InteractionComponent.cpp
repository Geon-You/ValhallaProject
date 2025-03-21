// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InteractionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Interface/ActorTypeInterface.h"
#include "Others/Enums.h"
#include "Character/ValhallaPlayerCharacter.h"
#include "Interface/VehicleInterface.h"

#include "Others/Debug.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UInteractionComponent::DoInteraction(AActor* InActor)
{
	if (InActor->IsValidLowLevel())
	{
		if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(InActor))
		{
			if (ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
			{
				if (GetOwnerRole() == ROLE_Authority)
				{
					VehicleInteraction(InActor);
				}
				else
				{
					Server_VehicleInteraction(InActor);
				}
			}
		}
	}
	else
	{

	}
}

void UInteractionComponent::VehicleInteraction(AActor* InActor)
{
	ITeamInterface* VehicleTeamInterface = Cast<ITeamInterface>(InActor);
	ITeamInterface* OwnerTeamInterface = Cast<ITeamInterface>(GetOwner());

	check(VehicleTeamInterface && OwnerTeamInterface);

	// 팀이 다르다면 탑승 못 함
	if (VehicleTeamInterface->GetActorTeamType() != EPlayerTeamType::None && VehicleTeamInterface->GetActorTeamType() != OwnerTeamInterface->GetActorTeamType())
	{
		return;
	}

	// 비어있는 자리를 리턴 받아서 탑승
	IVehicleInterface* VehicleInterface = Cast<IVehicleInterface>(InActor);
	if (!VehicleInterface || VehicleInterface->GetSeat() == ESeatType::NoSeat)
	{
		// nullptr거나 자리가 없다면 탑승 못 함
		return;
	}

	if (VehicleInterface && VehicleInterface->GetSeat() == ESeatType::Primary)
	{
		// 조종석에 빙의
		GetInTheCarPrimarySeat(InActor);
		VehicleInterface->SetPrimarySeat(GetOwner());
	}
	else if (VehicleInterface && VehicleInterface->GetSeat() == ESeatType::Secondary)
	{
		// 조수석에 탑승
		GetInTheCarSecondarySeat(InActor);
		VehicleInterface->SetSecondarySeat(GetOwner());
	}
}

void UInteractionComponent::GetInTheCarPrimarySeat(AActor* InActor)
{
	if (AValhallaPlayerCharacter* Player = GetOwner<AValhallaPlayerCharacter>())
	{
		Player->SetDrivingVehicle(InActor);
		Player->SetPlayerCurrentState(EPlayerCurrentState::OnBoard);
		Player->SetPlayerSeatType(ESeatType::Primary);
		Player->OnRep_PlayerSeatType();

		Player->GetInTheCar();
	}
}

void UInteractionComponent::GetInTheCarSecondarySeat(AActor* InActor)
{
	if (AValhallaPlayerCharacter* Player = GetOwner<AValhallaPlayerCharacter>())
	{
		Player->SetDrivingVehicle(InActor);
		Player->SetPlayerCurrentState(EPlayerCurrentState::OnBoard);
		Player->SetPlayerSeatType(ESeatType::Secondary);
		Player->OnRep_PlayerSeatType();
		Player->SetCameraDistance();
	}
}

void UInteractionComponent::Server_VehicleInteraction_Implementation(AActor* InActor)
{
	VehicleInteraction(InActor);
}

