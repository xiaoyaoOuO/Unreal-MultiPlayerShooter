// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Blast/BlasterComponents/CombatComponent.h"
#include "Blast/HUD/OverHeadWidget.h"
#include "Blast/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/NetworkObjectList.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;	//Natural length of the spring arm when there are no collisions
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//SocketName: The name of the socket at the end of the spring arm (looking back towards the spring arm origin)
	FollowCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	// If true, this Pawn's yaw will be updated to match the Controller's ControlRotation yaw, if controlled by a PlayerController.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//If true, rotate the Character toward the direction of acceleration, using RotationRate as the rate of rotation change. Overrides

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);

	OverlappingWeapon = nullptr;

    ACharacter::GetMovementComponent()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aiming",IE_Pressed,this,&ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aiming",IE_Released,this,&ABlasterCharacter::AImButtonReleased);
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire",IE_Released,this,&ABlasterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAxis("Look Up / Down",this,&ABlasterCharacter::LookUp);
	PlayerInputComponent->BindAxis("Move Forward / Backward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("Move Right / Left",this,&ABlasterCharacter::MoveRight);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter,AO_Yaw,COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABlasterCharacter,TurningInPlace,COND_SkipOwner);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		bool bAming = CombatComponent->bAiming;
		FName SlotName = bAming ? "RifleAim":"RifleHip";
		AnimInstance->Montage_JumpToSection(SlotName);
		UE_LOG(LogTemp,Warning,TEXT("PlayingMontage"));
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller && Value)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller && Value)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AImButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	//should turn
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;	
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw,0.f,DeltaTime,3.f);
		AO_Yaw = Interp_AO_Yaw;
		if (abs(AO_Yaw) < 10.f)
		{
			this->StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}
	// if (!HasAuthority() && !IsLocallyControlled())
	// {
	// 	FString message = "";
	// 	switch (TurningInPlace)
	// 	{
	// 	case ETurningInPlace::ETIP_NotTurning:
	// 		message = "not turning";
	// 		break;
	// 	case ETurningInPlace::ETIP_Right:
	// 		message = "right";
	// 		break;
	// 	case ETurningInPlace::ETIP_Left:
	// 		message = "left";
	// 		break;
	// 	}
	// 	UE_LOG(LogTemp, Warning,TEXT("other client %s") , *message);
	// 	UE_LOG(LogTemp, Warning,TEXT("other client %f") , AO_Yaw);
	// }
	// if (!HasAuthority() && IsLocallyControlled())
	// {
	// 	FString message = "";
	// 	switch (TurningInPlace)
	// 	{
	// 	case ETurningInPlace::ETIP_NotTurning:
	// 		message = "not turning";
	// 		break;
	// 	case ETurningInPlace::ETIP_Right:
	// 		message = "right";
	// 		break;
	// 	case ETurningInPlace::ETIP_Left:
	// 		message = "left";
	// 		break;
	// 	}
	// 	UE_LOG(LogTemp, Warning,TEXT("my client %s") , *message);
	// 	UE_LOG(LogTemp, Warning,TEXT("my client %f") , AO_Yaw);
	// }
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (!CombatComponent || CombatComponent->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed ==0 && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		if (HasAuthority() || IsLocallyControlled())
		{
			AO_Yaw = DeltaAimRotation.Yaw;
		}
		bUseControllerRotationYaw = true;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}

		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	
	if (AO_Pitch>90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f,360.f);
		FVector2D OutRange(-90.f,0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pitch);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bAiming;
}

AWeapon* ABlasterCharacter::Get_EquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}




