// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/StatusComponent.h"
#include "Net/UnrealNetwork.h"

#include "Others/Structs.h"

#include "Others/Debug.h"

UStatusComponent::UStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	/*
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableRef(TEXT("/Script/Engine.DataTable'/Game/_Valhalla/Data/Status/DT_PlayerStatus.DT_PlayerStatus'"));
	if (DataTableRef.Object)
	{
		StatusDataTable = DataTableRef.Object;
	}
	*/

	SetIsReplicated(true);

	CurrentLevel = 1;
	MaxLevel = 20;

	RegenTime = 1.f;
}

void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();


}

void UStatusComponent::PostInitProperties()
{
	Super::PostInitProperties();


}

void UStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatusComponent, MaxHealth);
	DOREPLIFETIME(UStatusComponent, MaxMana);
	DOREPLIFETIME(UStatusComponent, HealthRegen);
	DOREPLIFETIME(UStatusComponent, ManaRegen);
	DOREPLIFETIME(UStatusComponent, PhysicalAttack);
	DOREPLIFETIME(UStatusComponent, MagicalAttack);
	DOREPLIFETIME(UStatusComponent, PhysicalDefense);
	DOREPLIFETIME(UStatusComponent, MagicalDefense);
	DOREPLIFETIME(UStatusComponent, AttackSpeed);
	DOREPLIFETIME(UStatusComponent, MoveSpeed);
	DOREPLIFETIME(UStatusComponent, Sight);
	DOREPLIFETIME(UStatusComponent, MaxExp);
	DOREPLIFETIME(UStatusComponent, ExpGain);
	DOREPLIFETIME(UStatusComponent, CurrentLevel);
	DOREPLIFETIME(UStatusComponent, MaxLevel);
}

void UStatusComponent::Initialize()
{
	ensure(StatusDataTable);

	// 현재 레벨인 1레벨에 맞게 초기화
	SetStatus(CurrentLevel);
}

void UStatusComponent::SetStatus(const uint8 InCharacterLevel)
{
	// 레벨에 맞는 데이터 행을 불러와서 스탯에 적용
	FStatusDataTable* FoundedRow = StatusDataTable->FindRow<FStatusDataTable>(FName(FString::FromInt(InCharacterLevel)), TEXT("Cannot find DataTable"));
	if (FoundedRow) 
	{
		MaxHealth = FoundedRow->MaxHealth;
		MaxMana = FoundedRow->MaxMana;
		HealthRegen = FoundedRow->HealthRegen;
		ManaRegen = FoundedRow->ManaRegen;

		PhysicalAttack = FoundedRow->PhysicalAttack;
		MagicalAttack = FoundedRow->MagicalAttack;
		PhysicalDefense = FoundedRow->PhysicalDefense;
		MagicalDefense = FoundedRow->MagicalDefense;
		AttackSpeed = FoundedRow->AttackSpeed;

		MoveSpeed = FoundedRow->MoveSpeed;
		Sight = FoundedRow->Sight;

		MaxExp = FoundedRow->MaxExp;
		ExpGain = FoundedRow->ExpGain;
	}
}

void UStatusComponent::PrintStatus()
{
	Debug::Print(TEXT("MaxLevel"), MaxLevel, 5.f);
	Debug::Print(TEXT("ExpGain"), ExpGain, 5.f);
	Debug::Print(TEXT("MaxExp"), MaxExp, 5.f);
	Debug::Print(TEXT("Sight"), Sight, 5.f);
	Debug::Print(TEXT("MoveSpeed"), MoveSpeed, 5.f);
	Debug::Print(TEXT("AttackSpeed"), AttackSpeed, 5.f);
	Debug::Print(TEXT("MagicalDefence"), MagicalDefense, 5.f);
	Debug::Print(TEXT("PhysicalDefence"), PhysicalDefense, 5.f);
	Debug::Print(TEXT("MagicalAttack"), MagicalAttack, 5.f);
	Debug::Print(TEXT("PhysicalAttack"), PhysicalAttack, 5.f);
	Debug::Print(TEXT("ManaRegen"), ManaRegen, 5.f);
	Debug::Print(TEXT("HealthRegen"), HealthRegen, 5.f);
	Debug::Print(TEXT("MaxMana"), MaxMana, 5.f);
	Debug::Print(TEXT("MaxHealth"), MaxHealth, 5.f);
	Debug::Print(TEXT("Level"), CurrentLevel, 5.f);
}

void UStatusComponent::LevelUp()
{
	// 레벨 업을 하면 레벨에 맞게 스탯 설정
	// 레벨 업을 할 때 최대 레벨을 넘을 수 없음
	if (CurrentLevel <= MaxLevel - 1)
	{
		CurrentLevel += 1;

		SetStatus(CurrentLevel);
	}
}

void UStatusComponent::Regeneration()
{
	/*
	if (CurrentHealth == MaxHealth && CurrentMana == MaxMana)
	{
		return;
	}

	if (CurrentHealth == 0 && CurrentMana == 0)
	{
		return;
	}

	if (CurrentHealth < MaxHealth)
	{
		// TODO : 체력 리젠 공식으로 적용시키기
		CurrentHealth = FMath::Clamp(CurrentHealth + HealthRegen, CurrentHealth, MaxHealth);
		//Debug::Print(TEXT("CurrentHealth"), CurrentHealth, 4.f);
	}

	if (CurrentMana < MaxMana)
	{
		// TODO : 마나 리젠 공식으로 적용시키기
		CurrentMana = FMath::Clamp(CurrentMana + ManaRegen, CurrentMana, MaxMana);
		//Debug::Print(TEXT("CurrentMana"), CurrentMana, 4.f);
	}
	*/
}

void UStatusComponent::BindRegen()
{
	GetWorld()->GetTimerManager().SetTimer(RegenTimer, this, &UStatusComponent::Regeneration, RegenTime, true);
}

void UStatusComponent::StopRegen()
{
	GetWorld()->GetTimerManager().PauseTimer(RegenTimer);
}

void UStatusComponent::ReStartRegen()
{
	GetWorld()->GetTimerManager().UnPauseTimer(RegenTimer);
}

