
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Structs.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FStatusDataTable : public FTableRowBase
{
	GENERATED_BODY()

	// Base Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = BaseStat)
	float MaxHealth;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = BaseStat)
	float MaxMana;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = BaseStat)
	float HealthRegen;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = BaseStat)
	float ManaRegen;


	// Combat Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float PhysicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float MagicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float PhysicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float MagicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float AttackSpeed;


	// Util Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UtilStat)
	float MoveSpeed;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UtilStat)
	float Sight;


	// EXP Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = ExpStat)
	float MaxExp;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = ExpStat)
	float ExpGain;
};

USTRUCT(Blueprintable, BlueprintType)
struct FWeaponStatusDataTable : public FTableRowBase
{
	GENERATED_BODY()

	// Combat Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float PhysicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float MagicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float PhysicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float MagicalDefense;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = CombatStat)
	float AttackSpeed;
};
