// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Others/Enums.h"
#include "ValhallaWeaponBase.generated.h"

class UBoxComponent;
class UWeaponData;

UCLASS()
class VALHALLAPROJECT_API AValhallaWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AValhallaWeaponBase();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


#pragma region Component Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	TObjectPtr<UBoxComponent> WeaponCollisionBox;

public:
	FORCEINLINE UBoxComponent* GetWeaponCollision() const { return WeaponCollisionBox; }

private:
	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion

#pragma region Weapon Status Variable Section
protected:
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "WeaponData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> WeaponStatusData;

	// Weapon Status
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float WeaponPhysicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float WeaponMagicalAttack;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float AdditionalPhysicalDefence;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float AdditionalMagicalDefence;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponStatus", meta = (AllowPrivateAccess = "true"), Replicated)
	float WeaponAttackSpeed;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponLevel", meta = (AllowPrivateAccess = "true"), Replicated)
	uint8 WeaponLevel;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "WeaponData|WeaponLevel", meta = (AllowPrivateAccess = "true"), Replicated)
	uint8 WeaponMaxLevel;

public:
	FORCEINLINE float GetWeaponPhysicalAttack() const { return WeaponPhysicalAttack; }
	FORCEINLINE float GetWeaponMagicalAttack() const { return WeaponMagicalAttack; }
	FORCEINLINE float GetAdditionalPhysicalDefence() const { return AdditionalPhysicalDefence; }
	FORCEINLINE float GetAdditionalMagicalDefence() const { return AdditionalMagicalDefence; }
	FORCEINLINE float GetWeaponAttackSpeed() const { return WeaponAttackSpeed; }
#pragma endregion

#pragma region Weapon Section
public:
	void Initialize();

protected:
	void SetWeaponStatus(const uint8 InWeaponLevel);	// 무기의 레벨에 맞는 스탯 초기화

	bool IsSameTeam(AActor* InOwnerActor, AActor* InOtherActor);	// 적팀에게만 데미지를 적용하기 위해 같은 팀인지 적팀인지를 판단하는 함수, 근데 어차피 팀 검사를 데미지 받는 쪽에서 하고 있어서 여기선 안 씀

public:
	void WeaponUpgrade();	// 무기 레벨 업, 근데 이제 무기 강화 부분이 기획에서 빠지고 아티팩트로 한다고 했으니까 사용 안 할 예정
	void ClearHitActors();	// 무기에 적중한 액터 목록을 초기화하는 함수

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponData", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> HitActors;	// 무기에 닿은 적들을 저장하는 배열, 무기에 닿았다고 해서 한 명의 적에게 여러 번의 데미지가 적용되는 걸 방지하기 위해

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "WeaponData", meta = (AllowPrivateAccess = "true"))
	EPlayerWeaponType WeaponType;

public:
	EPlayerWeaponType& GetWeaponType() { return WeaponType; }
#pragma endregion
};
