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
	void SetWeaponStatus(const uint8 InWeaponLevel);	// ������ ������ �´� ���� �ʱ�ȭ

	bool IsSameTeam(AActor* InOwnerActor, AActor* InOtherActor);	// �������Ը� �������� �����ϱ� ���� ���� ������ ���������� �Ǵ��ϴ� �Լ�, �ٵ� ������ �� �˻縦 ������ �޴� �ʿ��� �ϰ� �־ ���⼱ �� ��

public:
	void WeaponUpgrade();	// ���� ���� ��, �ٵ� ���� ���� ��ȭ �κ��� ��ȹ���� ������ ��Ƽ��Ʈ�� �Ѵٰ� �����ϱ� ��� �� �� ����
	void ClearHitActors();	// ���⿡ ������ ���� ����� �ʱ�ȭ�ϴ� �Լ�

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponData", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> HitActors;	// ���⿡ ���� ������ �����ϴ� �迭, ���⿡ ��Ҵٰ� �ؼ� �� ���� ������ ���� ���� �������� ����Ǵ� �� �����ϱ� ����

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "WeaponData", meta = (AllowPrivateAccess = "true"))
	EPlayerWeaponType WeaponType;

public:
	EPlayerWeaponType& GetWeaponType() { return WeaponType; }
#pragma endregion
};
