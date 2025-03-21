// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ValhallaCharacterHUD.generated.h"

/**
 * 
 */
UCLASS()
class VALHALLAPROJECT_API UValhallaCharacterHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthBar(float InCurrentHealth, float InMaxHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void SetExpBar(float InCurrentExp, float InMaxExp);

	UFUNCTION(BlueprintImplementableEvent)
	void SetLevel(uint8 InNewLevel);

	UFUNCTION(BlueprintImplementableEvent)
	void SetGold(int32 InNewGold);
};
