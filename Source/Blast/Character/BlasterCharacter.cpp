// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Blast/Blast.h"
#include "Blast/BlasterComponents/CombatComponent.h"
#include "Blast/GameMode/BlasterGameMode.h"
#include "Blast/PlayerState/BlasterPlayerState.h"
#include "Blast/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


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

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

	OverlappingWeapon = nullptr;

    ACharacter::GetMovementComponent()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	bShouldElim = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
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

void ABlasterCharacter::UpdateHealthHUD()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetBlasterPlayerHealth(CurrentHealth,MaxHealth);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DissolveMaterialInstanceDynamic)
	{
		DissolveMaterialInstanceDynamic->SetScalarParameterValue("Dissolve",DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	if (DissolveTimeline && DissolveCurve)
	{
		DissolveTimelineTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTimelineTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	UpdateHealthHUD();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	this->TimeSinceLastMovementReplication = 0.f;
	SimProxiesTurn();
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBot)
	{
		ElimBot->DestroyComponent();
	}
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

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter,AO_Yaw,COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABlasterCharacter,TurningInPlace,COND_SkipOwner);
	DOREPLIFETIME(ABlasterCharacter,CurrentHealth);
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
		bool bAiming = CombatComponent->bAiming;
		FName SlotName = bAiming ? "RifleAim":"RifleHip";
		AnimInstance->Montage_JumpToSection(SlotName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SlotName = "HitFront";
		AnimInstance->Montage_JumpToSection(SlotName);
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
}

void ABlasterCharacter::Calculate_AO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	
	if (AO_Pitch>90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f,360.f);
		FVector2D OutRange(-90.f,0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pitch);
	}
}

float ABlasterCharacter::Calculate_Speed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_CurrentHealth()
{
	UpdateHealthHUD();
	PlayHitReactMontage();
	if (CurrentHealth <= 0.f)
	{
		Elim();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage,0.f,MaxHealth);
	UpdateHealthHUD();
	PlayHitReactMontage();
	if (CurrentHealth <= 0.f)
	{
		if (ABlasterGameMode* BlasterGameMode =  Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode()))
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			BlasterGameMode->CharacterElim(this,BlasterPlayerController,Cast<ABlasterPlayerController>(InstigatedBy));
			GetWorldTimerManager().SetTimer(
				RespawnTimer,
				this,
				&ABlasterCharacter::RespawnTimerFinished,
				RespawnDelay,
				false
			);
		}
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (!CombatComponent || CombatComponent->EquippedWeapon == nullptr) return;
	float Speed = Calculate_Speed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed ==0 && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	Calculate_AO_Pitch();
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

void ABlasterCharacter::HideCharacterWhenCameraClose()
{
	if (!IsLocallyControlled() || FollowCamera == nullptr) return;

	float CameraDistance = FVector::Distance(FollowCamera->GetComponentLocation(),GetActorLocation());
	if (CameraDistance < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon)
		{
			// CombatComponent->EquippedWeapon->Get_WeaponMesh()->SetVisibility(false);
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon)
		{
			// CombatComponent->EquippedWeapon->Get_WeaponMesh()->SetVisibility(true);
		}
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;

	float Speed = Calculate_Speed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyLastRotationFrame = ProxyRotationFrame;
	ProxyRotationFrame = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationFrame,ProxyLastRotationFrame).Yaw;
	if (ProxyYaw > TurnThreshold)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (ProxyYaw < -TurnThreshold)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	if (ElimMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ElimMontage);
		}
	}
}

void ABlasterCharacter::RespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterGameMode && BlasterPlayerController)
	{
		BlasterGameMode->RespawnCharacter(this,BlasterPlayerController);
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->UpdatePlayerScore(BlasterPlayerState->GetScore());
		}
	}
}

void ABlasterCharacter::Elim()
{
	bShouldElim = true;
	PlayElimMontage();

	//掉落武器
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}

	//禁用移动，禁用碰撞
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	
	//溶解特效
	if (DissolveMaterialInstance)
	{
		DissolveMaterialInstanceDynamic = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this,FName("Dissolve"));
		DissolveMaterialInstanceDynamic->SetScalarParameterValue(FName("Dissolve"),-0.2f);
		DissolveMaterialInstanceDynamic->SetScalarParameterValue(FName("Glow"),200.f);
		GetMesh()->SetMaterial(0,DissolveMaterialInstanceDynamic);
	}
	StartDissolve();

	if (ElimBotParticle)
	{
		FVector Player = GetActorLocation();
		FVector BotSpawnLocation = FVector(Player.X,Player.Y,Player.Z + 200.f);
		ElimBot = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotParticle,
			BotSpawnLocation
		);
	}
	if (BotSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),BotSoundCue,GetActorLocation());
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

FVector ABlasterCharacter::Get_HitResult()
{
	if (CombatComponent == nullptr) return FVector();
	return CombatComponent->HitTargetPoint;
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > .25f)
		{
			OnRep_ReplicatedMovement();
		}
		Calculate_AO_Pitch();
	}
	HideCharacterWhenCameraClose();
	PollInit();
}




