// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ValhallaCharacterBase.h"
#include "Interface/CombatInterface.h"
#include "Interface/GoldInterface.h"
#include "Interface/SeatInterface.h"
#include "ValhallaPlayerCharacter.generated.h"

DECLARE_DELEGATE_TwoParams(FOnHealthChangedSignature, float, float);
DECLARE_DELEGATE_TwoParams(FOnExpChangedSignature, float, float);
DECLARE_DELEGATE_OneParam(FOnLevelChangedSignature, uint8);
DECLARE_DELEGATE_OneParam(FOnGoldChangedSignature, int32);

class USpringArmComponent;
class UValhallaCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UStatusComponent;
class UInteractionComponent;
class UCombatComponent;
class AValhallaWeaponBase;
class UWeaponData;
class UValhallaCharacterHUD;

/**
 * 
 */
UCLASS()
class VALHALLAPROJECT_API AValhallaPlayerCharacter : public AValhallaCharacterBase, public ICombatInterface, public IGoldInterface, public ISeatInterface
{
	GENERATED_BODY()
	
public:
	AValhallaPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	// Network
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


#pragma region Controller Section
private:
	void SetControlledPawn(AController* InNewController);
#pragma endregion


#pragma region Component Section
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UValhallaCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UValhallaCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UStatusComponent* GetStatusComponent() const { return StatusComponent; }
	FORCEINLINE UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
#pragma endregion


#pragma region InputSection
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* QSkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ESkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RSkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RightMouseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LeftMouseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Keyboard1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Keyboard2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShiftAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interaction();
	void QSkillPressed();
	void ESkillPressed();
	void RSkillPressed();
	void RightMouseClicked();
	void LeftMouseClicked();
	void Keyboard1Pressed();
	void Keyboard2Pressed();
	void ShiftPressed();
	void ShiftReleased();

	UFUNCTION(Client, Reliable)
	void Client_GetSubsystem(AController* InNewController);
#pragma endregion


#pragma region Interface Section
public:
	// ���� ����
	virtual void ToggleWeaponCollision(bool IsCollisionEnable) override;
	// ���� ����
	virtual float GetCharacterAttackPower() override;
	virtual float GetCharacterAttackSpeed() override;
	// ��� ȹ�� ����
	virtual void GainGold(int32 InGoldGivenAmount) override;	// �������̽��� ���� ��带 ȹ���ϴ� �Լ�
	virtual bool IsPlayerCharacter() override;

	// ���� ����
	virtual void SetSeatType(ESeatType InNewSeatType) override;	// �������� ž�� ������, �������� ž�� ������ �����ϴ� Setter
	virtual void SeatInitialize() override;	// �������⿡�� �������� �� ĳ���� �ݸ��� ���� �ʱ�ȭ, ���� ���� �ʱ�ȭ
	virtual void VehicleInitialize() override;	// �������⿡�� �������� �� ���� ������ �ʱ�ȭ
#pragma endregion


#pragma region Weapon Section
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponData> WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AValhallaWeaponBase> WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"), Replicated)
	TObjectPtr<AValhallaWeaponBase> CharacterWeapon;

private:
	AValhallaWeaponBase* SpawnPlayerWeapon(TSubclassOf<AValhallaWeaponBase> InWeaponClass);

public:
	FORCEINLINE AValhallaWeaponBase* GetPlayerWeapon() const { return CharacterWeapon; }
	FORCEINLINE const float GetFinalAttackSpeed();
#pragma endregion


#pragma region Status Section
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentMana)
	float CurrentMana;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentExp)
	float CurrentExp;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentGold)
	int32 CurrentGold;

	UFUNCTION()
	void OnRep_CurrentMana();

	UFUNCTION()
	void OnRep_CurrentExp();

	UFUNCTION()
	void OnRep_CurrentGold();

	virtual void InitializeHealthAndMana() override;	// ���� ������ �� ü�� ���� �ʱ�ȭ
	virtual void ResetHealthAndMana() override;	// ������ ���� �� ü�� ���� �ʱ�ȭ

	// �Ű������� ���� �������� �����ϴ� �Լ�
	void DamagedCharacterHealth(const float InDamage);

	bool IsFirstInitialize;
#pragma endregion


#pragma region Gain Exp And Gold Section
// ��ȭ ȹ�� ���� Section
public:
	// ����ġ�� ��� ȹ���� ó���ϴ� �Լ�
	void GainExpAndGold(const float InExp, const int32 InGold);
#pragma endregion


#pragma region Respawn Section
protected:
	virtual void Die() override;	// �׾��� �� ����Ǵ� �Լ�(������ ������ ���)

public:
	UFUNCTION(BlueprintImplementableEvent)
	void DieNextLogic();	// �������Ʈ���� ����

private:
	UFUNCTION(Client, Reliable)
	void SetInputOnOff(const bool IsInputOff);

	UFUNCTION(BlueprintCallable)
	void RespawnCharacter();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Respawn, meta = (AllowPrivateAccess = "true"), Replicated)
	FTransform RespawnTransform;	// ĳ���� ������ ��ġ
#pragma endregion


#pragma region Debug Section
public:
	UFUNCTION(BlueprintCallable)
	void PrintCurrentHealthMana();

	UFUNCTION(BlueprintCallable)
	void PrintWeaponStatus();

	UFUNCTION(BlueprintCallable)
	void PrintCurrentExp();
#pragma endregion


#pragma region Vehicle Section
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Vehicle, meta = (AllowPrivateAccess = "true"), Replicated)
	TObjectPtr<AActor> DrivingVehicle;	// ĳ���Ͱ� ���� ž�� ���� ���������� ������

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Vehicle, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_PlayerSeatType)
	ESeatType PlayerSeatType;	// ĳ���Ͱ� ���� ����������, ������������ ��Ÿ���� ����

	FORCEINLINE void SetPlayerSeatType(const ESeatType InNewSeatType) { PlayerSeatType = InNewSeatType; }
	FORCEINLINE void SetDrivingVehicle(AActor* InNewDrivingVehicle) { DrivingVehicle = InNewDrivingVehicle; }

	UFUNCTION()
	void OnRep_PlayerSeatType();

	void GetInTheCar();	// ž�� ����
	void GetOffTheCar();	// ���������� ���� ����, (���������� ���� ������ ���������ʿ� ����)

	UFUNCTION(Server, Reliable)
	void Server_GetOffTheCar();

	UFUNCTION(NetMulticast, Reliable)
	void MC_GetOffTheCar(FVector InNewLocation);

	void SetCameraDistance();

	UPROPERTY(EditAnywhere, Category = Camera)
	float NormalCameraDistance;

	UPROPERTY(EditAnywhere, Category = Camera)
	float OnBoardCameraDistance;
#pragma endregion


#pragma region Delegate Section
private:
	UFUNCTION(NetMulticast, Reliable)
	void OnCharacterHealthChanged(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController);

	UFUNCTION(Client, Reliable)
	void OnHealthChanged(float InCurrentHealth, float InMaxHealth);

	UFUNCTION(Client, Reliable)
	void OnExpChanged(float InCurrentExp, float InMaxExp);

	UFUNCTION(Client, Reliable)
	void OnLevelChanged(uint8 InNewLevel);

	UFUNCTION(Client, Reliable)
	void OnGoldChanged(int32 InNewLevel);

	FOnHealthChangedSignature OnHealthChangedDelegate;
	FOnExpChangedSignature OnExpChangedDelegate;
	FOnLevelChangedSignature OnLevelChangedDelegate;
	FOnGoldChangedSignature OnGoldChangedDelegate;
#pragma endregion


#pragma region Character HUD Section
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TObjectPtr<UValhallaCharacterHUD> CharacterHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widgets)
	TSubclassOf<UValhallaCharacterHUD> CharacterHUDClass;
#pragma endregion
};
