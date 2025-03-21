// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Vehicle/ValhallaVehicle.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosVehicleMovementComponent.h"

#include "Widget/ValhallaHealthWidget.h"
#include "Interface/GoldInterface.h"

#include "Others/Debug.h"

AValhallaVehicle::AValhallaVehicle()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	/*
	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleCollision"));
	VehicleCollision->SetupAttachment(GetRootComponent());
	VehicleCollision->OnComponentBeginOverlap.AddDynamic(this, &AValhallaVehicle::OnCollisionBoxBeginOverlap);

	VehicleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VehicleWidgetComponent"));
	VehicleWidgetComponent->SetupAttachment(GetRootComponent());
	*/

	// Camera Section
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	TeamType = EPlayerTeamType::None;

	ActorType = EActorType::Vehicle;

	MaxHealth = 2000.f;
	VehicleDefense = 10.f;

	GoldGivenAmount = 150;
}

void AValhallaVehicle::BeginPlay()
{
	if (HasAuthority())
	{
		InitializeStatus();
	}

	/*
	UValhallaHealthWidget* HealthWidget = Cast<UValhallaHealthWidget>(VehicleWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		Debug::Print(TEXT("HealthWidget"), 5.f);
		OnVehicleHealthChangedDelegate.BindUObject(HealthWidget, &UValhallaHealthWidget::SetHealthBar);
	}
	*/
}

void AValhallaVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	Client_GetSubsystem(NewController);
}

void AValhallaVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(VehicleMoveForwardAction, ETriggerEvent::Triggered, this, &AValhallaVehicle::VehicleMoveForwardTriggered);
		EnhancedInputComponent->BindAction(VehicleMoveForwardAction, ETriggerEvent::Started, this, &AValhallaVehicle::VehicleMoveForwardStarted);
		EnhancedInputComponent->BindAction(VehicleMoveForwardAction, ETriggerEvent::Canceled, this, &AValhallaVehicle::VehicleMoveForwardStarted);

		EnhancedInputComponent->BindAction(VehicleTurnRightAction, ETriggerEvent::Triggered, this, &AValhallaVehicle::VehicleTurnRightTriggered);
		EnhancedInputComponent->BindAction(VehicleTurnRightAction, ETriggerEvent::Started, this, &AValhallaVehicle::VehicleTurnRightStarted);
		EnhancedInputComponent->BindAction(VehicleTurnRightAction, ETriggerEvent::Completed, this, &AValhallaVehicle::VehicleTurnRightCompleted);
		EnhancedInputComponent->BindAction(VehicleTurnRightAction, ETriggerEvent::Canceled, this, &AValhallaVehicle::VehicleTurnRightCompleted);

		EnhancedInputComponent->BindAction(VehicleLookAction, ETriggerEvent::Triggered, this, &AValhallaVehicle::VehicleLook);

		Debug::Print(TEXT("Set InputComponent"), 5.f);
	}
}

void AValhallaVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaVehicle, CurrentHealth);
}

float AValhallaVehicle::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 중립 상태에선 무적
	if (EventInstigator == nullptr || TeamType == EPlayerTeamType::None)
	{
		return 0.f;
	}

	if (ITeamInterface* TeamInterface = Cast<ITeamInterface>(EventInstigator->GetPawn()))
	{
		// 적 팀의 데미지가 아니라면 데미지 적용 X
		if (TeamInterface->GetActorTeamType() == TeamType)
		{
			return 0.f;
		}
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float Damage = FMath::Clamp(DamageAmount - VehicleDefense, 0, DamageAmount);

	if (HasAuthority())
	{
		if (Damage > 0)
		{
			float NewCurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, CurrentHealth);
			CurrentHealth = NewCurrentHealth;

			// 공격을 가한 상대가 플레이어라면
			if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(EventInstigator->GetPawn<APawn>()))
			{
				if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player)
				{
					OnVehicleHealthChanged(CurrentHealth, MaxHealth, EventInstigator);
				}
			}
		}

		Debug::Print(TEXT("CurrentHealth"), CurrentHealth, 5.f);

		if (CurrentHealth <= 0)
		{
			// 파괴되면 공격을 가한 상대방에게 골드를 줌
			if (IGoldInterface* InstigatorCharacter = Cast<IGoldInterface>(EventInstigator->GetPawn()))
			{
				// 상대방이 미니언이 아니고 캐릭터라면 골드를 줌
				if (InstigatorCharacter->IsPlayerCharacter())
				{
					InstigatorCharacter->GainGold(GoldGivenAmount);
				}
			}

			DestroyVehicle();
		}
	}

	return Damage;
}

void AValhallaVehicle::Destroyed()
{
	Super::Destroyed();

	// 파괴 연출 재생
}

void AValhallaVehicle::Client_GetSubsystem_Implementation(AController* InController)
{
	if (APlayerController* NewPlayerController = Cast<APlayerController>(InController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			check(VehicleMappingContext);
			Subsystem->AddMappingContext(VehicleMappingContext, 0);

			Debug::Print(TEXT("Mapping"), 5.f);
		}
	}
}

void AValhallaVehicle::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 공성 병기랑 부딪히면 캐릭터나 미니언이 밀려나도록
	if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(OtherActor))
	{
		if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Minion)
		{
			
		}
	}
}

void AValhallaVehicle::VehicleMoveForwardTriggered(const FInputActionValue& Value)
{
	float ThrottleRate = Value.Get<float>();

	//Debug::Print(TEXT("F"), ThrottleRate, 5.f);

	GetVehicleMovementComponent()->SetThrottleInput(ThrottleRate);
	GetVehicleMovementComponent()->SetBrakeInput(0.f);
}

void AValhallaVehicle::VehicleMoveForwardStarted(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetYawInput(0.f);
	GetVehicleMovementComponent()->SetThrottleInput(0.f);
}

void AValhallaVehicle::VehicleTurnRightTriggered(const FInputActionValue& Value)
{
	float TurnRate = Value.Get<float>();

	//Debug::Print(TEXT("R"), TurnRate, 5.f);

	GetVehicleMovementComponent()->SetYawInput(TurnRate);
}

void AValhallaVehicle::VehicleTurnRightStarted(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetThrottleInput(0.f);
}

void AValhallaVehicle::VehicleTurnRightCompleted(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetYawInput(0.f);
}

void AValhallaVehicle::VehicleLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	//Debug::Print(TEXT("Look"), LookAxisVector.X, 5.f);
	//Debug::Print(TEXT("Look"), LookAxisVector.Y, 5.f);

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AValhallaVehicle::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0)
	{
		Debug::Print(TEXT("Vehicle Destroyed"), 5.f);
	}
}

void AValhallaVehicle::InitializeStatus()
{
	CurrentHealth = MaxHealth;
}

EPlayerTeamType& AValhallaVehicle::GetActorTeamType()
{
	return TeamType;
}

EActorType& AValhallaVehicle::GetActorTypeTag()
{
	return ActorType;
}

void AValhallaVehicle::DestroyVehicle()
{
	Destroy();
}

void AValhallaVehicle::OnVehicleHealthChanged_Implementation(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController)
{
	OnVehicleHealthChangedDelegate.ExecuteIfBound(InCurrentHealth, InMaxHealth, InInstigatorController);
}
