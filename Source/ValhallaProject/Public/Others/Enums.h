#pragma once

UENUM(BlueprintType)
enum class EPlayerTeamType : uint8
{
	None = 0,
	Red,
	Blue,
	Neutral
};

UENUM(BlueprintType)
enum class EPlayerWeaponType : uint8
{
	Axe = 0,
	Bow
};

UENUM(BlueprintType)
enum class EPlayerCurrentState : uint8
{
	Alive = 0,
	Dead,
	OnBoard
};

UENUM(BlueprintType)
enum class EActorType : uint8
{
	Player = 0,
	Minion,
	Construction,
	Turret,
	Vehicle,
	InteractableItem
};

UENUM(BlueprintType)
enum class ESeatType : uint8
{
	None = 0,
	Primary,
	Secondary,
	NoSeat
};

