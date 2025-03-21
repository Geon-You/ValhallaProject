// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ValhallaPlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

#include "Component/ValhallaCameraComponent.h"
#include "Component/StatusComponent.h"
#include "Component/InteractionComponent.h"
#include "Component/CombatComponent.h"
#include "Actors/Weapon/ValhallaWeaponBase.h"
#include "Data/Weapon/WeaponData.h"
#include "Player/ValhallaPlayerController.h"
#include "Interface/VehicleInterface.h"
#include "Widget/ValhallaHealthWidget.h"
#include "Widget/ValhallaCharacterHUD.h"

#include "Others/Debug.h"

AValhallaPlayerCharacter::AValhallaPlayerCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->GravityScale = 1.75f;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Camera Section
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	FollowCamera = CreateDefaultSubobject<UValhallaCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Status Section
	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

	// Interaction Section
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	// Combat Section
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	CharacterHealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterHealthWidgetComponent"));
	CharacterHealthWidgetComponent->SetupAttachment(RootComponent);

	TeamType = EPlayerTeamType::None;

	// Character Type Section
	ActorType = EActorType::Player;

	// Weapon Section
	CharacterWeapon = nullptr;

	CurrentGold = 0;
	GoldGivenAmount = 300;

	DrivingVehicle = nullptr;
	PlayerSeatType = ESeatType::None;

	IsFirstInitialize = true;

	OnBoardCameraDistance = 1600.f;
	NormalCameraDistance = 500.f;

	// 깃허브 테스트
}

void AValhallaPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 무기 스폰
	ensure(WeaponType);
	if (HasAuthority())	// 서버라면
	{
		CharacterWeapon = SpawnPlayerWeapon(WeaponType);	// 무기 스폰
		if (CharacterWeapon)
		{
			FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
			CharacterWeapon->AttachToComponent(GetMesh(), AttachmentRule, "WeaponSocket");	// 무기 장착
		}
	}

	// 상대방에게 보여질 체력바 델리게이트와 바인딩
	UValhallaHealthWidget* HealthWidget = Cast<UValhallaHealthWidget>(CharacterHealthWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		OnCharacterHealthChangedDelegate.BindUObject(HealthWidget, &UValhallaHealthWidget::SetHealthBar);
	}

	// 체력, 마나 리젠 시작
	//StatusComponent->BindRegen();

	// 자기 자신의 UI를 델리게이트와 바인딩
	CharacterHUD = Cast<UValhallaCharacterHUD>(CreateWidget(GetWorld()->GetFirstPlayerController(), CharacterHUDClass));
	if (CharacterHUD)
	{
		OnHealthChangedDelegate.BindUObject(CharacterHUD, &UValhallaCharacterHUD::SetHealthBar);
		OnHealthChangedDelegate.ExecuteIfBound(CurrentHealth, StatusComponent->GetMaxHealth());

		OnExpChangedDelegate.BindUObject(CharacterHUD, &UValhallaCharacterHUD::SetExpBar);
		OnExpChangedDelegate.ExecuteIfBound(CurrentExp, StatusComponent->GetMaxExp());

		OnLevelChangedDelegate.BindUObject(CharacterHUD, &UValhallaCharacterHUD::SetLevel);
		OnLevelChangedDelegate.ExecuteIfBound(StatusComponent->GetCurrentLevel());

		OnGoldChangedDelegate.BindUObject(CharacterHUD, &UValhallaCharacterHUD::SetGold);
		OnGoldChangedDelegate.ExecuteIfBound(CurrentGold);
	}

	// Possess 먼저 -> BeginPlay 호출
}

void AValhallaPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Base Key Binding
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AValhallaPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AValhallaPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::Interaction);

		// Skill Key Binding
		EnhancedInputComponent->BindAction(QSkillAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::QSkillPressed);
		EnhancedInputComponent->BindAction(ESkillAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::ESkillPressed);
		EnhancedInputComponent->BindAction(RSkillAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::RSkillPressed);
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::RightMouseClicked);
		EnhancedInputComponent->BindAction(LeftMouseAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::LeftMouseClicked);

		// Item Key Binding
		EnhancedInputComponent->BindAction(Keyboard1Action, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::Keyboard1Pressed);
		EnhancedInputComponent->BindAction(Keyboard2Action, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::Keyboard2Pressed);

		// Sprint Key Binding
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AValhallaPlayerCharacter::ShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AValhallaPlayerCharacter::ShiftReleased);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Canceled, this, &AValhallaPlayerCharacter::ShiftReleased);
	}
}

void AValhallaPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void AValhallaPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();


}

void AValhallaPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버에서 초기화
	SetControlledPawn(NewController);

	// 처음 한 번만 스탯들 초기화
	if (IsFirstInitialize)
	{
		StatusComponent->Initialize();
		InitializeHealthAndMana();

		IsFirstInitialize = false;
	}

	Client_GetSubsystem(NewController);
}

void AValhallaPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 초기화
}

void AValhallaPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AValhallaPlayerCharacter, CurrentMana);
	DOREPLIFETIME(AValhallaPlayerCharacter, CurrentExp);
	DOREPLIFETIME(AValhallaPlayerCharacter, CurrentGold);

	DOREPLIFETIME(AValhallaPlayerCharacter, CharacterWeapon);

	DOREPLIFETIME(AValhallaPlayerCharacter, RespawnTransform);

	DOREPLIFETIME(AValhallaPlayerCharacter, PlayerSeatType);
	DOREPLIFETIME(AValhallaPlayerCharacter, DrivingVehicle);
}

float AValhallaPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 데미지를 가한 찰나에 상대방이 죽으면 데미지를 안 입도록 
	// ex.) 포탑이 미사일을 쏘고 죽으면 그 미사일에는 데미지를 입지 않음
	// 또한 죽은 상태라면 데미지를 입지 않음
	// 탑승 상태에선 데미지를 입지 않음
	if (EventInstigator == nullptr || PlayerCurrentState == EPlayerCurrentState::Dead || PlayerCurrentState == EPlayerCurrentState::OnBoard)
	{
		return 0.f;
	}

	// 같은 팀의 공격이라면 데미지를 입지 않음
	if (ITeamInterface* TeamInterface = Cast<ITeamInterface>(EventInstigator->GetPawn()))
	{
		EPlayerTeamType Team = TeamInterface->GetActorTeamType();

		// 적 팀의 데미지가 아니라면 데미지 적용 X
		if (TeamInterface->GetActorTeamType() == TeamType)
		{
			return 0.f;
		}
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 서버에서 호출
	if (HasAuthority())
	{
		// 데미지 적용
		DamagedCharacterHealth(DamageAmount);

		// 공격을 가한 상대가 플레이어이거나 공성병기라면
		if (IActorTypeInterface* ActorTypeInterface = Cast<IActorTypeInterface>(EventInstigator->GetPawn<APawn>()))
		{
			if (ActorTypeInterface->GetActorTypeTag() == EActorType::Player || ActorTypeInterface->GetActorTypeTag() == EActorType::Vehicle)
			{
				OnCharacterHealthChanged(CurrentHealth, StatusComponent->GetMaxHealth(), EventInstigator);
			}
		}

		// 체력이 0 이하로 떨어지면 죽음
		if (IsDead(CurrentHealth))
		{
			Die();	// 죽는 로직 실행
			OnRep_PlayerCurrentState();	// 서버에서도 사망 리액션 재생

			// 죽으면 공격을 가한 상대방에게 경험치를 줌
			if (AValhallaCharacterBase* InstigatorCharacter = Cast<AValhallaCharacterBase>(EventInstigator->GetPawn()))
			{
				// 상대방이 미니언이 아니고 캐릭터라면 경험치와 골드를 줌
				if (InstigatorCharacter->GetActorType() == EActorType::Player)
				{
					if (AValhallaPlayerCharacter* PlayerCharacter = Cast<AValhallaPlayerCharacter>(InstigatorCharacter))
					{
						float Exp = StatusComponent->GetExpGain();
						PlayerCharacter->GainExpAndGold(Exp, GoldGivenAmount);
					}
				}
			}
		}
	}

	return DamageAmount;
}

void AValhallaPlayerCharacter::SetControlledPawn(AController* InNewController)
{
	if (AValhallaPlayerController* NewPlayerController = Cast<AValhallaPlayerController>(InNewController))
	{
		NewPlayerController->ControlledPawn = this;
	}
}

void AValhallaPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AValhallaPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AValhallaPlayerCharacter::Interaction()
{
	if (PlayerCurrentState == EPlayerCurrentState::OnBoard)
	{
		// 차량 탑승 상태에선 상호작용이 차량에서 내리기 기능으로 작동하도록
		if (HasAuthority())
		{
			GetOffTheCar();
		}
		else
		{
			Server_GetOffTheCar();
		}
	}
	else
	{
		InteractionComponent->DoInteraction(FollowCamera->GetTraceHitResult());
	}
}

void AValhallaPlayerCharacter::QSkillPressed()
{
	//Debug::Print(TEXT("Q Skill Pressed"), 5.f);
}

void AValhallaPlayerCharacter::ESkillPressed()
{
	//Debug::Print(TEXT("E Skill Pressed"), 5.f);
}

void AValhallaPlayerCharacter::RSkillPressed()
{
	//Debug::Print(TEXT("R Skill Pressed"), 5.f);
}

void AValhallaPlayerCharacter::RightMouseClicked()
{
	//Debug::Print(TEXT("Right Mouse Button Clicked"), 5.f);
}

void AValhallaPlayerCharacter::LeftMouseClicked()
{
	CombatComponent->NormalAttack(CharacterWeapon);
}

void AValhallaPlayerCharacter::Keyboard1Pressed()
{
	//Debug::Print(TEXT("Keyboard 1 Pressed"), 5.f);

	if (DrivingVehicle)
	{
		// 공성병기 스킬
	}
	else
	{
		// 캐릭터 스킬
	}
}

void AValhallaPlayerCharacter::Keyboard2Pressed()
{
	//Debug::Print(TEXT("Keyboard 2 Pressed"), 5.f);
}

void AValhallaPlayerCharacter::ShiftPressed()
{
	//Debug::Print(TEXT("Shift Pressed"), 5.f);
}

void AValhallaPlayerCharacter::ShiftReleased()
{
	//Debug::Print(TEXT("Shift Released"), 5.f);
}

void AValhallaPlayerCharacter::Client_GetSubsystem_Implementation(AController* InNewController)
{
	if (AValhallaPlayerController* NewPlayerController = Cast<AValhallaPlayerController>(InNewController))
	{
		NewPlayerController->ControlledPawn = this;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			check(DefaultMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

AValhallaWeaponBase* AValhallaPlayerCharacter::SpawnPlayerWeapon(TSubclassOf<AValhallaWeaponBase> InWeaponClass)
{
	const FTransform SpawnTransform(GetActorLocation());
	AValhallaWeaponBase* SpawnedWeapon = GetWorld()->SpawnActorDeferred<AValhallaWeaponBase>(InWeaponClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (SpawnedWeapon)
	{
		SpawnedWeapon->Initialize();
		SpawnedWeapon->FinishSpawning(SpawnTransform);
		return SpawnedWeapon;
	}

	return nullptr;
}

void AValhallaPlayerCharacter::ToggleWeaponCollision(bool IsCollisionEnable)
{
	Super::ToggleWeaponCollision(IsCollisionEnable);

	if (IsCollisionEnable)
	{
		CharacterWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		CharacterWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CharacterWeapon->ClearHitActors();
	}
}

float AValhallaPlayerCharacter::GetCharacterAttackPower()
{
	return StatusComponent->GetPhysicalAttack();
}

float AValhallaPlayerCharacter::GetCharacterAttackSpeed()
{
	float StatusAttackSpeed = StatusComponent->GetAttackSpeed();
	float WeaponAttackSpeed = CharacterWeapon->GetWeaponAttackSpeed();

	return StatusAttackSpeed * WeaponAttackSpeed;
}

void AValhallaPlayerCharacter::GainGold(int32 InGoldGivenAmount)
{
	CurrentGold += InGoldGivenAmount;
}

bool AValhallaPlayerCharacter::IsPlayerCharacter()
{
	if (ActorType == EActorType::Player)
	{
		return true;
	}

	return false;
}

void AValhallaPlayerCharacter::SetSeatType(ESeatType InNewSeatType)
{
	PlayerSeatType = InNewSeatType;
}

void AValhallaPlayerCharacter::SeatInitialize()
{
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
	DetachFromActor(DetachmentRules);

	// ECC_GameTraceChannel10은 공성추 채널
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Block);
	// ECC_GameTraceChannel7는 SiegeWeapon 채널
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel7, ECollisionResponse::ECR_Block);
	// ECC_GameTraceChannel11은 사석포 채널
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel11, ECollisionResponse::ECR_Block);
	// ECC_GameTraceChannel9는 CannonWeapon 채널
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECollisionResponse::ECR_Block);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	PlayerCurrentState = EPlayerCurrentState::Alive;
}

void AValhallaPlayerCharacter::VehicleInitialize()
{
	DrivingVehicle = nullptr;

	CameraBoom->TargetArmLength = NormalCameraDistance;
}

const float AValhallaPlayerCharacter::GetFinalAttackSpeed()
{
	float StatusAttackSpeed = StatusComponent->GetAttackSpeed();
	float WeaponAttackSpeed = CharacterWeapon->GetWeaponAttackSpeed();

	return StatusAttackSpeed * WeaponAttackSpeed;
}

void AValhallaPlayerCharacter::OnRep_CurrentMana()
{

}

void AValhallaPlayerCharacter::OnRep_CurrentExp()
{

}

void AValhallaPlayerCharacter::OnRep_CurrentGold()
{

}

void AValhallaPlayerCharacter::GainExpAndGold(const float InExp, const int32 InGold)
{
	float NewExp = CurrentExp + InExp;

	if (StatusComponent->GetMaxExp() <= NewExp)	// 경험치가 최대 경험치를 넘었을 때
	{
		float RemainingExp = NewExp - StatusComponent->GetMaxExp();
		CurrentExp = RemainingExp;
		StatusComponent->LevelUp();	// 레벨 업

		// 자신의 HUD에 반영
		OnLevelChangedDelegate.ExecuteIfBound(StatusComponent->GetCurrentLevel());
		OnLevelChanged(StatusComponent->GetCurrentLevel());

		OnExpChangedDelegate.ExecuteIfBound(CurrentExp, StatusComponent->GetMaxExp());
		OnExpChanged(CurrentExp, StatusComponent->GetMaxExp());

		OnHealthChangedDelegate.ExecuteIfBound(CurrentHealth, StatusComponent->GetMaxHealth());
		OnHealthChanged(CurrentHealth, StatusComponent->GetMaxHealth());
	}
	else
	{
		CurrentExp = NewExp;

		// 자신의 HUD에 반영
		OnExpChangedDelegate.ExecuteIfBound(CurrentExp, StatusComponent->GetMaxExp());
		OnExpChanged(CurrentExp, StatusComponent->GetMaxExp());
	}

	CurrentGold += InGold;

	// 자신의 HUD에 반영
	OnGoldChangedDelegate.ExecuteIfBound(CurrentGold);
	OnGoldChanged(CurrentGold);
}

void AValhallaPlayerCharacter::Die()
{
	Super::Die();

	// 캐릭터 상태 변경
	PlayerCurrentState = EPlayerCurrentState::Dead;

	// 콜리전 감지되지 않도록 끄기
	SetCollisionOnOff(false);

	// 입력 끄기
	SetInputOnOff(false);

	//StatusComponent->StopRegen();

	// 다음 로직으로 계속
	DieNextLogic();
}

void AValhallaPlayerCharacter::SetInputOnOff_Implementation(const bool IsInputOff)
{
	if (!IsInputOff)
	{
		APlayerController* CharacterController = Cast<APlayerController>(GetController());
		DisableInput(CharacterController);
	}
	else
	{
		APlayerController* CharacterController = Cast<APlayerController>(GetController());
		EnableInput(CharacterController);
	}
}

void AValhallaPlayerCharacter::RespawnCharacter()
{
	// 체력, 마나 초기화
	ResetHealthAndMana();

	// 체력, 마나 리젠 다시 시작
	//StatusComponent->ReStartRegen();

	// 리스폰 장소로 옮김
	SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::ResetPhysics);

	// 콜리전 다시 활성화
	SetCollisionOnOff(true);

	// 입력 다시 활성화
	SetInputOnOff(true);

	// 플레이어 상태 변경
	PlayerCurrentState = EPlayerCurrentState::Alive;

	// 공격을 실행 가능한 상태로 만들기
	CombatComponent->SetIsAttackingNow(false);
}

void AValhallaPlayerCharacter::PrintCurrentHealthMana()
{
	Debug::Print(TEXT("CurrentMana"), CurrentMana, 5.f);
	Debug::Print(TEXT("CurrentHealth"), CurrentHealth, 5.f);
}

void AValhallaPlayerCharacter::PrintWeaponStatus()
{
	Debug::Print(TEXT("WeaponMagicalAttack"), CharacterWeapon->GetWeaponMagicalAttack(), 5.f);
	Debug::Print(TEXT("WeaponPhysicalAttack"), CharacterWeapon->GetWeaponPhysicalAttack(), 5.f);
}

void AValhallaPlayerCharacter::PrintCurrentExp()
{
	Debug::Print(TEXT("CurrentGold"), CurrentGold, 5.f);
	Debug::Print(TEXT("MaxExp"), StatusComponent->GetMaxExp(), 5.f);
	Debug::Print(TEXT("CurrentExp"), CurrentExp, 5.f);
}

void AValhallaPlayerCharacter::OnRep_PlayerSeatType()
{
	if (PlayerSeatType == ESeatType::Primary)
	{
		IVehicleInterface* VehicleInterface = Cast<IVehicleInterface>(DrivingVehicle);

		// ECC_GameTraceChannel10은 공성추 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel11은 사석포 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel11, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel7는 SiegeWeapon 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel7, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel7는 CannonWeapon 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECollisionResponse::ECR_Ignore);

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		AttachToComponent(VehicleInterface->GetVehicleMesh(), AttachmentRules, TEXT("PrimarySeat"));	// 운전석에 캐릭터를 부착
	}
	else if (PlayerSeatType == ESeatType::Secondary)
	{
		IVehicleInterface* VehicleInterface = Cast<IVehicleInterface>(DrivingVehicle);

		// ECC_GameTraceChannel10은 공성추 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel11은 사석포 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel11, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel7는 SiegeWeapon 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel7, ECollisionResponse::ECR_Ignore);
		// ECC_GameTraceChannel7는 CannonWeapon 채널
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECollisionResponse::ECR_Ignore);

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		AttachToComponent(VehicleInterface->GetVehicleMesh(), AttachmentRules, TEXT("SecondarySeat"));	// 보조석에 캐릭터를 부착
	}
	else if (PlayerSeatType == ESeatType::None)
	{
		// 캐릭터를 하차 상태로 초기화
		SeatInitialize();
	}
}

void AValhallaPlayerCharacter::GetInTheCar()
{
	// 공성병기에 빙의
	if (APawn* DrivingPawn = Cast<APawn>(DrivingVehicle))
	{
		GetController()->Possess(DrivingPawn);
	}
}

void AValhallaPlayerCharacter::GetOffTheCar()
{
	// 보조석에서 하차할 때
	PlayerSeatType = ESeatType::None;
	SeatInitialize();

	if (IVehicleInterface* VehicleInterface = Cast<IVehicleInterface>(DrivingVehicle))
	{
		FVector GetOffLocation = VehicleInterface->VehicleSecondarySeatGetOffLocation();
		MC_GetOffTheCar(GetOffLocation);

		VehicleInterface->SetSecondarySeat(nullptr);
	}

	VehicleInitialize();
}

void AValhallaPlayerCharacter::SetCameraDistance()
{
	CameraBoom->TargetArmLength = OnBoardCameraDistance;
}

void AValhallaPlayerCharacter::OnCharacterHealthChanged_Implementation(float InCurrentHealth, float InMaxHealth, AController* InInstigatorController)
{
	OnCharacterHealthChangedDelegate.ExecuteIfBound(InCurrentHealth, InMaxHealth, InInstigatorController);
}

void AValhallaPlayerCharacter::Server_GetOffTheCar_Implementation()
{
	GetOffTheCar();
}

void AValhallaPlayerCharacter::MC_GetOffTheCar_Implementation(FVector InNewLocation)
{
	SetActorLocation(InNewLocation, false, nullptr, ETeleportType::ResetPhysics);
}

void AValhallaPlayerCharacter::InitializeHealthAndMana()
{
	CurrentHealth = StatusComponent->GetMaxHealth();
	CurrentMana = StatusComponent->GetMaxMana();
	CurrentExp = 0.f;
	CurrentGold = 0;
}

void AValhallaPlayerCharacter::ResetHealthAndMana()
{
	CurrentHealth = StatusComponent->GetMaxHealth();
	CurrentMana = StatusComponent->GetMaxMana();

	// 자기 UI에도 깎인 체력 반영
	OnHealthChangedDelegate.ExecuteIfBound(CurrentHealth, StatusComponent->GetMaxHealth());
	OnHealthChanged(CurrentHealth, StatusComponent->GetMaxHealth());
}

void AValhallaPlayerCharacter::DamagedCharacterHealth(const float InDamage)
{
	float Damage = FMath::Clamp(InDamage - StatusComponent->GetPhysicalDefense(), 0, InDamage);

	float NewCurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, CurrentHealth);
	CurrentHealth = NewCurrentHealth;

	// 자기 UI에도 깎인 체력 반영
	OnHealthChangedDelegate.ExecuteIfBound(CurrentHealth, StatusComponent->GetMaxHealth());
	OnHealthChanged(CurrentHealth, StatusComponent->GetMaxHealth());
}

void AValhallaPlayerCharacter::OnHealthChanged_Implementation(float InCurrentHealth, float InMaxHealth)
{
	// 자기 UI에도 깎인 체력 반영
	OnHealthChangedDelegate.ExecuteIfBound(CurrentHealth, StatusComponent->GetMaxHealth());
}

void AValhallaPlayerCharacter::OnExpChanged_Implementation(float InCurrentExp, float InMaxExp)
{
	// 자기 UI에 변경된 경험치 반영
	OnExpChangedDelegate.ExecuteIfBound(CurrentExp, StatusComponent->GetMaxExp());
}

void AValhallaPlayerCharacter::OnLevelChanged_Implementation(uint8 InNewLevel)
{
	// 자기 UI에 변경된 레벨 반영
	OnLevelChangedDelegate.ExecuteIfBound(StatusComponent->GetCurrentLevel());
}

void AValhallaPlayerCharacter::OnGoldChanged_Implementation(int32 InNewLevel)
{
	// 자기 UI에 변경된 골드 반영
	OnGoldChangedDelegate.ExecuteIfBound(CurrentGold);
}
