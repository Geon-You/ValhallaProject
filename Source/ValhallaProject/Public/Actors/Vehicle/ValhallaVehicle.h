// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Interface/TeamInterface.h"
#include "Interface/ActorTypeInterface.h"
#include "Others/Enums.h"
#include "ValhallaVehicle.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnVehicleHealthChangedSignature, float, float, AController*);

class UBoxComponent;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class VALHALLAPROJECT_API AValhallaVehicle : public AWheeledVehiclePawn, public ITeamInterface, public IActorTypeInterface
{
	GENERATED_BODY()
	
public:
	AValhallaVehicle();

protected:
	virtual void BeginPlay() override;
	// Network
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

protected:
	UFUNCTION(Client, Reliable)
	void Client_GetSubsystem(AController* InController);


#pragma region Component Section
	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Vehicle, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<UBoxComponent> VehicleCollision;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<UWidgetComponent> VehicleWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

private:
	UFUNCTION()
	void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion


#pragma region Input Section
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> VehicleMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> VehicleMoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> VehicleTurnRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> VehicleLookAction;

	void VehicleMoveForwardTriggered(const FInputActionValue& Value);
	void VehicleMoveForwardStarted(const FInputActionValue& Value);
	void VehicleTurnRightTriggered(const FInputActionValue& Value);
	void VehicleTurnRightStarted(const FInputActionValue& Value);
	void VehicleTurnRightCompleted(const FInputActionValue& Value);
	void VehicleLook(const FInputActionValue& Value);
#pragma endregion


#pragma region Status Section
protected:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"))
	float VehicleDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Status, meta = (AllowPrivateAccess = "true"))
	int32 GoldGivenAmount;

public:
	UFUNCTION()
	void OnRep_CurrentHealth();

private:
	void InitializeStatus();
#pragma endregion


#pragma region Enum Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Team, meta = (AllowPrivateAccess = "true"))
	EPlayerTeamType TeamType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Enum, meta = (AllowPrivateAccess = "true"))
	EActorType ActorType;

public:
	FORCEINLINE const EPlayerTeamType& GetTeamType() { return TeamType; }
	FORCEINLINE const EActorType& GetActorType() { return ActorType; }

	FORCEINLINE void SetTeamType(EPlayerTeamType InNewTeamType) { TeamType = InNewTeamType; }
#pragma endregion


#pragma region Interface Section
public:
	virtual EPlayerTeamType& GetActorTeamType() override;
	virtual EActorType& GetActorTypeTag() override;
#pragma endregion


#pragma region Destroy Section
private:
	void DestroyVehicle();
#pragma endregion


#pragma region Delegate Section
public:
	FOnVehicleHealthChangedSignature OnVehicleHealthChangedDelegate;

	UFUNCTION(NetMulticast, Reliable)
	void OnVehicleHealthChanged(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController);
#pragma endregion
};
