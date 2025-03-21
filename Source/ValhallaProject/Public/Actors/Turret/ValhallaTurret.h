// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Others/Enums.h"
#include "Interface/TeamInterface.h"
#include "Interface/ActorTypeInterface.h"
#include "ValhallaTurret.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnTurretHealthChangedSignature, float, float, AController*);

class UBoxComponent;
class ATurretAIController;
class AValhallaTurretBullet;
class UWidgetComponent;

UCLASS()
class VALHALLAPROJECT_API AValhallaTurret : public APawn, public ITeamInterface, public IActorTypeInterface
{
	GENERATED_BODY()

public:
	AValhallaTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;


#pragma region Component Section
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> BulletSpawnTransform;	// 불 발사체 생성 위치

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> TurretWidgetComponent;

private:
	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion


#pragma region Status Section
protected:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"))
	float TurretDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Status, meta = (AllowPrivateAccess = "true"))
	int32 GoldGivenAmount;

public:
	UFUNCTION()
	void OnRep_CurrentHealth();

private:
	void InitializeStatus();

#pragma endregion


#pragma region AI Section
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AIController, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ATurretAIController> TurretController;
#pragma endregion


#pragma region Enum Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Enums, meta = (AllowPrivateAccess = "true"))
	EPlayerTeamType TeamType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Enums, meta = (AllowPrivateAccess = "true"))
	EActorType ActorType;

public:
	FORCEINLINE EPlayerTeamType& GetTeamType() { return TeamType; }
	FORCEINLINE EActorType& GetActorType() { return ActorType; }
#pragma endregion


#pragma region Fire Bullet Section
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fire, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AValhallaTurretBullet> TurretBulletClass;


	UFUNCTION(Server, Reliable, BlueprintCallable)
	void BulletFire(AActor* InTargetActor);
#pragma endregion


#pragma region Destroy Section
private:
	void DestroyTurret();
#pragma endregion


#pragma region Interface Section
public:
	virtual EPlayerTeamType& GetActorTeamType() override;
	virtual EActorType& GetActorTypeTag() override;
#pragma endregion


#pragma region Delegate Section
public:
	FOnTurretHealthChangedSignature OnTurretHealthChangedDelegate;

	UFUNCTION(NetMulticast, Reliable)
	void OnTurretHealthChanged(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController);
#pragma endregion
};
