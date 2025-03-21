// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ValhallaPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class VALHALLAPROJECT_API AValhallaPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AValhallaPlayerController();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Controll, meta = (AllowPrivateAccess = "true"), Replicated)
	TObjectPtr<APawn> ControlledPawn;
};
