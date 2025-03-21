// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

class UDataTable;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VALHALLAPROJECT_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatusComponent();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


#pragma region Getter & Setter Function Section
public:
	FORCEINLINE const float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE const float GetMaxMana() const { return MaxMana; }

	FORCEINLINE const float GetHealthRegen() const { return HealthRegen; }
	FORCEINLINE const float GetManaRegen() const { return ManaRegen; }

	FORCEINLINE const float GetPhysicalAttack() const { return PhysicalAttack; }
	FORCEINLINE const float GetPhysicalDefense() const { return PhysicalDefense; }

	FORCEINLINE const float GetAttackSpeed() const { return AttackSpeed; }

	FORCEINLINE const float GetExpGain() const { return ExpGain; }
	FORCEINLINE const float GetMaxExp() const { return MaxExp; }

	FORCEINLINE const uint8 GetCurrentLevel() const { return CurrentLevel; }
#pragma endregion


#pragma region Status Section
// 스테이터스 함수 관련 Section
public:
	void Initialize();	// 스탯 초기화
	void LevelUp();	// 레벨 업 하면 업한 레벨에 맞게 스탯 다시 설정

private:
	void SetStatus(const uint8 InCharacterLevel);	// 실질적으로 스탯이 설정되는 함수

public:
	UFUNCTION(BlueprintCallable)
	void PrintStatus();
#pragma endregion


#pragma region Health & Mana Regen Section
private:
	void Regeneration();

public:
	void BindRegen();
	void StopRegen();
	void ReStartRegen();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusData|Regen", meta = (AllowPrivateAccess = "true"))
	FTimerHandle RegenTimer;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|Regen", meta = (AllowPrivateAccess = "true"))
	float RegenTime;
#pragma endregion


#pragma region Status Variable Section
private:
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|DataTable", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> StatusDataTable;

	// Base Status
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|BaseStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|BaseStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxMana;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|BaseStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float HealthRegen;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|BaseStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float ManaRegen;

	// Combat Status
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|CombatStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float PhysicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|CombatStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MagicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|CombatStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float PhysicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|CombatStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MagicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|CombatStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float AttackSpeed;

	// Util Status
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|UtilStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MoveSpeed;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|UtilStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float Sight;

	// EXP Status
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|ExpStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxExp;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|ExpStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float ExpGain;

	// Level Status
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|LevelStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	uint8 CurrentLevel;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "StatusData|LevelStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	uint8 MaxLevel;
#pragma endregion
};
