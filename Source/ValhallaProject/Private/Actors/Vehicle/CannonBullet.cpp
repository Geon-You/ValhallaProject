// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Vehicle/CannonBullet.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "Interface/ActorTypeInterface.h"

ACannonBullet::ACannonBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttackPartMesh"));
	SetRootComponent(BulletMesh);
	BulletMesh->OnComponentHit.AddDynamic(this, &ACannonBullet::OnBulletMeshHit);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 1000.f;
	ProjectileMovementComp->MaxSpeed = 1000.f;
	ProjectileMovementComp->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovementComp->ProjectileGravityScale = 1.f;

	TeamType = EPlayerTeamType::None;

	BulletDamage = 200.f;

	HitEffectScale = FVector(1.f, 1.f, 1.f);

	HitLocation = FVector(0.f, 0.f, 0.f);
}

void ACannonBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACannonBullet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACannonBullet, HitLocation);
}

void ACannonBullet::Destroyed()
{
	Super::Destroyed();

	// 서버랑 클라이언트에서 모두 이펙트 재생
	//PlayHitEffect();
}

void ACannonBullet::OnBulletMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 서버에서만 동작하도록
	if (HasAuthority())
	{
		// 터지는 소리도 넣기

		AController* OwnerPawnController = GetOwner<APawn>()->GetController();

		if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(OtherActor))
		{
			if (ActorTypeInterface->GetActorTypeTag() == EActorType::Construction || ActorTypeInterface->GetActorTypeTag() == EActorType::Turret)
			{
				float FinalDamage = BulletDamage * 1.5f;
				UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, OwnerPawnController, this, nullptr);
			}
			else if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Minion)
			{
				float FinalDamage = BulletDamage;
				UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, OwnerPawnController, this, nullptr);
			}
			else if (ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
			{
				float FinalDamage = BulletDamage * 1.2f;
				UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, OwnerPawnController, this, nullptr);
			}
		}

		HitLocation = Hit.ImpactPoint;

		PlayHitEffect(HitLocation);
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		AfterHit();
	}
}

void ACannonBullet::PlayHitEffect(FVector InHitLocation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ProjectileHitEffect, InHitLocation, GetActorRotation(), HitEffectScale);
}

void ACannonBullet::OnRep_HitLocation()
{
	PlayHitEffect(HitLocation);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

