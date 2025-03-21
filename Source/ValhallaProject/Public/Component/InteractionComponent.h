// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VALHALLAPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;

	
#pragma region Interaction Section
public:
	void DoInteraction(AActor* InActor);

private:
	void VehicleInteraction(AActor* InActor);

	UFUNCTION(Server, Reliable)
	void Server_VehicleInteraction(AActor* InActor);

	void GetInTheCarPrimarySeat(AActor* InActor);
	void GetInTheCarSecondarySeat(AActor* InActor);
#pragma endregion
};
