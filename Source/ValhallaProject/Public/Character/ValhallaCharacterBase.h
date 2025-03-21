// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Others/Enums.h"
#include "Interface/WeaponInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Interface/TeamInterface.h"
#include "Interface/ActorTypeInterface.h"
#include "ValhallaCharacterBase.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnCharacterHealthChangedSignature, float, float, AController*);

class UAIPerceptionStimuliSourceComponent;
class UWidgetComponent;

UCLASS()
class VALHALLAPROJECT_API AValhallaCharacterBase : public ACharacter, public IWeaponInterface, public IGenericTeamAgentInterface, public ITeamInterface, public IActorTypeInterface
{
	GENERATED_BODY()

public:
	AValhallaCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Network
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	// Interface
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId((uint8)TeamType); }


#pragma region Component Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> AIPerceptionStimuliSourceComponent;
#pragma endregion


#pragma region Enum Section
// 팀 종류나 캐릭터 종류, 죽은 상태 등등 열거형 Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"), ReplicatedUsing = OnRep_TeamType)
	EPlayerTeamType TeamType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Enum, meta = (AllowPrivateAccess = "true"))
	EActorType ActorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_PlayerCurrentState)
	EPlayerCurrentState PlayerCurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> CharacterHealthWidgetComponent;

public:
	UFUNCTION()
	void OnRep_PlayerCurrentState();

	UFUNCTION()
	void OnRep_TeamType();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetTeamCharacterMaterial(EPlayerTeamType InNewTeamType);

	FORCEINLINE const EPlayerTeamType& GetTeamType() const { return TeamType; }
	FORCEINLINE const EActorType& GetActorType() const { return ActorType; }
	FORCEINLINE const EPlayerCurrentState& GetPlayerCurrentState() const { return PlayerCurrentState; }

	FORCEINLINE void SetTeamType(EPlayerTeamType InPlayerTeamType) { TeamType = InPlayerTeamType; }
	FORCEINLINE void SetPlayerCurrentState(const EPlayerCurrentState InPlayerState) { PlayerCurrentState = InPlayerState; }
#pragma endregion


#pragma region Character Section
// 캐릭터 관련 변수, 함수 Section
protected:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Status, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Status, meta = (AllowPrivateAccess = "true"), Replicated)
	int32 GoldGivenAmount;

public:
	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void InitializeHealthAndMana();
	virtual void ResetHealthAndMana();
#pragma endregion


#pragma region Damage And Death Section
// 데미지와 죽음 관련 처리 Section
protected:
	virtual void Die();
	bool IsDead(const float InCurrentHealth);

	UFUNCTION(NetMulticast, Reliable)
	void SetCollisionOnOff(const bool IsCollisionOn);
#pragma endregion


#pragma region Montage Section
// 캐릭터에 필요한 몽타주 Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HitReactionMontage;
#pragma endregion


#pragma region Interface Section
// 상속받은 인터페이스 구현 Section
public:
	virtual EActorType& GetActorTypeTag() override;
	virtual EPlayerTeamType& GetActorTeamType() override;
	virtual void ToggleWeaponCollision(bool IsCollisionEnable) override;
#pragma endregion


#pragma region Delegate Section
	protected:
		FOnCharacterHealthChangedSignature OnCharacterHealthChangedDelegate;
#pragma endregion
};
