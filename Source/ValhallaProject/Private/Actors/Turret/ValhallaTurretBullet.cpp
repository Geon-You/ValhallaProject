// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Turret/ValhallaTurretBullet.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

#include "Interface/ActorTypeInterface.h"

#include "Others/Debug.h"

AValhallaTurretBullet::AValhallaTurretBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	ProjectileCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollisionSphere"));
	SetRootComponent(ProjectileCollisionSphere);

	ProjectileCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ProjectileCollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	ProjectileCollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);
	ProjectileCollisionSphere->OnComponentHit.AddDynamic(this, &AValhallaTurretBullet::OnCollisionSphereHit);

	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 700.f;
	ProjectileMovementComp->MaxSpeed = 900.f;
	ProjectileMovementComp->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	TeamType = EPlayerTeamType::None;

	BulletDamage = 100.f;

	BulletTargetActor = nullptr;

	DamageRadius = 100.f;

	HitEffectScale = FVector(1.f, 1.f, 1.f);
}

void AValhallaTurretBullet::BeginPlay()
{
	Super::BeginPlay();


}

void AValhallaTurretBullet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaTurretBullet, BulletTargetActor);
}

void AValhallaTurretBullet::Destroyed()
{
	Super::Destroyed();

	// 서버랑 클라이언트에서 모두 이펙트 재생
	PlayHitEffect();
}

void AValhallaTurretBullet::OnCollisionSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 서버에서만 동작하도록
	if (HasAuthority())
	{
		// 터지는 소리도 넣기

		TArray<AActor*> IgnoreActors;
		IgnoreActors.AddUnique(GetInstigator());

		AController* OwnerPawnController = GetOwner<APawn>()->GetController();

		UGameplayStatics::ApplyRadialDamage(this, BulletDamage, Hit.ImpactPoint, DamageRadius, nullptr, IgnoreActors, this, OwnerPawnController, true);

		//DrawDebugSphere(GetWorld(), Hit.Location, DamageRadius, 35.f, FColor::Red, false, 3.f, 0.f, 2.f);

		Destroy();
	}
}

void AValhallaTurretBullet::PlayHitEffect()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ProjectileHitEffect, GetActorLocation(), GetActorRotation(), HitEffectScale);
}

