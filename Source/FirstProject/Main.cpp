// Fill out your copyright notice in the Description page of Project Settings.

#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "Critter.h"
#include "ItemStorage.h"

//#include "WeaponContainerActor.h"


// Sets default values
AMain::AMain()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** Creates camera boom (pulls toward the player if there's a collision*/
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; /**Camera follows at this distance*/
	CameraBoom->bUsePawnControlRotation = true; /**Rotate arm base on the controller*/

	/**Create follow Camera*/
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	/**Attach camera to end the of the boom and let the boom adjust to match the controller orientation */
	FollowCamera->bUsePawnControlRotation = false;

	/**Set size for collision capsule*/
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	/**Don't roll with the controller rotates
	Let that just affect the camera */
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	/**Configure Character Movement*/
	GetCharacterMovement()->bOrientRotationToMovement = true;/**Character moves in the direction of input*/
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);/**... at this rotation rate*/
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;


	/**Base turn rates*/
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
	Coins = 0.f;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bLMBDown = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

	//bESCDown = false;



}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();


	MainPlayerController = Cast<AMainPlayerController>(GetController());

	//FString Map = GetWorld()->GetMapName();
	//Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	
	
		//LoadGameNoSwitch();
		//if (MainPlayerController)
		//{
		//	MainPlayerController->GameModeOnly();
		//}
	

}



// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}

		}
		else//**Shift key up*/
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0.f;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				if (bMovingForward || bMovingRight)
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}

		}
		else /** Shift Key up*/
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}

			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else /** Shift Key up*/
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);

		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		;

	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);

	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}



// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent)

		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpRate);
	//PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	//PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

}
void AMain::MoveForward(float Value)
{
	/**Find out which way is forward*/
	bMovingForward = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}

}

void AMain::MoveRight(float Value)
{
	/**Find out which way is forward*/
	bMovingRight = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;

	}
}

void AMain::TurnRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AMain::LookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return;


	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon)
	{
		Attack();
	}
}
	void AMain::LMBUp()
	{
		bLMBDown = false;
	}

	/*void AMain::ESCDown()
	{
		bESCDown = true;
	
		if (MainPlayerController->bPauseMenuVisible)
		{
			MainPlayerController->TogglePauseMenu();
		}
	}

	void AMain::ESCUp()
	{
		bESCDown = false;
	}*/

void AMain::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}


void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;

}

void AMain::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}


void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}


void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}


void AMain::ShowPickupLocations()
{
	for (auto Location : PickupLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Green, 10.f, .5f);
	}

	/**for (int32 i = 0; i < PickupLocations.Num(); i++)
{
	UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i],25.f, 8, FLinearColor::Green, 10.f, .5f);
}*/

}
void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			int32 Section = FMath::RandRange(0, 1);
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack_01"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 2.0f);
				AnimInstance->Montage_JumpToSection(FName("Attack_02"), CombatMontage);
				break;

			default:
				;
			}


		}
	}
}


void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}



void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;


}



float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* enemy = Cast<AEnemy>(DamageCauser);
			if (enemy)
			{
				enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AMain::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);


	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}
void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		//CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			//FString Level = LevelName.ToString();
			//UE_LOG(LogTemp, Warning, TEXT("CurrentLevel: %s"), *CurrentLevel)
			//	UE_LOG(LogTemp, Warning, TEXT("LevelName: %s"), *Level)
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}
		void AMain::SaveGame()
		{
			UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

			SaveGameInstance->CharacterStats.Health = Health;
			SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
			SaveGameInstance->CharacterStats.Stamina = Stamina;
			SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
			SaveGameInstance->CharacterStats.Coins = Coins;
			SaveGameInstance->CharacterStats.Location = GetActorLocation();
			SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

			FString MapName = GetWorld()->GetMapName();
			//UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName)
			MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
			//UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName)
			SaveGameInstance->CharacterStats.LevelName = MapName;
			//	UE_LOG(LogTemp, Warning, TEXT("SaveObject->CharacterStats.LevelName: %s"), *SaveObject->CharacterStats.LevelName)*/

			
			if (EquippedWeapon)
			{
				SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
				//SaveGameInstance->CharacterStats.bWeaponParticles = EquippedWeapon->bWeaponParticles;
			}

			UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
		}

		void AMain::LoadGame(bool SetPosition)
		{
			UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
			LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));


				Health = LoadGameInstance->CharacterStats.Health;
				MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;

				Stamina = LoadGameInstance->CharacterStats.Stamina;
				MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

				Coins = LoadGameInstance->CharacterStats.Coins;
			
				if (WeaponStorage)
				{

					AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
					if (Weapons)
					{
						FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

						if (Weapons->WeaponMap.Contains(WeaponName))

						{
							AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
							WeaponToEquip->Equip(this);
						}

					}
				}
			

				if (SetPosition)
				{
					SetActorLocation(LoadGameInstance->CharacterStats.Location);
					SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
				}

				SetMovementStatus(EMovementStatus::EMS_Normal);
				GetMesh()->bPauseAnims = false;
				GetMesh()->bNoSkeletonUpdate = false;
				
				/*if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
				{
					FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

					SwitchLevel(LevelName); 
				}*/
			
		}
		void AMain::LoadGameNoSwitch()
		{
			UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
			LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));


			

				Health = LoadGameInstance->CharacterStats.Health;
				MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;

				Stamina = LoadGameInstance->CharacterStats.Stamina;
				MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

				Coins = LoadGameInstance->CharacterStats.Coins;

				if (WeaponStorage)
				{

					AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
					if (Weapons)
					{
						FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

						if (Weapons->WeaponMap.Contains(WeaponName))

						{
							AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
							WeaponToEquip->Equip(this);
						}

					}
				}
				
			


			
		}

			
		
