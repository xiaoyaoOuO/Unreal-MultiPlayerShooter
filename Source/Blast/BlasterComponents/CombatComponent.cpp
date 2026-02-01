// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blast/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#define TRACE_LENGTH 80000.f

UCombatComponent::UCombatComponent()
{
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	BaseMoveSpeed = 600.f;
	AimingMoveSpeed = 400.f;
	bCanFire = true;
}

void UCombatComponent::MulticastFire_Implementation(FVector_NetQuantize HitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage();
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize HitTarget)
{
	MulticastFire(HitTarget);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}
	FVector2D CrosshairLocation = FVector2D(ViewPortSize.X / 2, ViewPortSize.Y / 2);

	// 屏幕坐标转换为世界坐标
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector StartLocation = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - StartLocation).Size();
			StartLocation += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		FVector EndLocation = StartLocation + CrosshairWorldDirection * TRACE_LENGTH;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			StartLocation,
			EndLocation,
			ECC_Visibility
		);
		if (bHit)
		{
			//TODO：目前是瞄准到敌人变红色，后续考虑命中再变红
			if (const AActor *HitActor = TraceHitResult.GetActor(); HitActor && HitActor->Implements<UInteractWithCrosshairsInterface>())
			{
				HUDPackage.CrosshairColor = FLinearColor::Red;
			}else
			{
				HUDPackage.CrosshairColor = FLinearColor::White;
			}
		}
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndLocation;
		}
	}
}

void UCombatComponent::UpdateHUD(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			/*
			 *计算准心扩散
			*/
			FVector2D SpeedRange = FVector2D(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector CharacterSpeed = Character->GetVelocity();
			CharacterSpeed.Z = 0.f;

			WalkSpeedFactor =  FMath::GetMappedRangeValueClamped(SpeedRange,FVector2D(0,1.f),CharacterSpeed.Size());
			
			//跳跃扩散更大
			if (Character->GetCharacterMovement()->IsFalling())
			{
				InAirFactor = FMath::FInterpTo(InAirFactor,2.25f,DeltaTime,2.25f);
			}else
			{
				InAirFactor = FMath::FInterpTo(InAirFactor,0.f,DeltaTime,30.f);
			}

			if (bAiming)
			{
				AimFactor = FMath::FInterpTo(AimFactor,0.58f,DeltaTime,5.f);
			}else
			{
				AimFactor = FMath::FInterpTo(AimFactor,0.f,DeltaTime,30.f);
			}

			ShootingFactor = FMath::FInterpTo(ShootingFactor,0.f,DeltaTime,40.f);
			HUDPackage.SpreadSize = 0.5f + WalkSpeedFactor + InAirFactor + ShootingFactor - AimFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	ZoomFOV = EquippedWeapon->ZoomFOV;
	ZoomSpeed = EquippedWeapon->ZoomSpeed;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,ZoomFOV,DeltaTime,ZoomSpeed);
	}else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,DefaultFOV,DeltaTime,ZoomSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
		DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character && EquippedWeapon)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;    //关闭随移动转向
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::StartFireDelay()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	FireDelay = EquippedWeapon->FireDelay;
	bCanFire = false;
	Character->GetWorldTimerManager().SetTimer(
		FireDelayTimer,
		this,
		&UCombatComponent::FireTimerFinish,
		FireDelay
	);
}

void UCombatComponent::FireTimerFinish()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomaticFire)
	{
		Fire();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		this->HitTargetPoint = HitResult.ImpactPoint;
		InterpFOV(DeltaTime);
		UpdateHUD(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Character == nullptr || Weapon ==nullptr) return;

	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket"))
	{
		HandSocket->AttachActor(Weapon, Character->GetMesh());
	}
	Weapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;    //关闭随移动转向
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;	//客户端预测
	Server_SetAiming(bIsAiming);	//服务端校验
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimingMoveSpeed : BaseMoveSpeed;
    }
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		ShootingFactor = 0.2f;
		ServerFire(HitTargetPoint);
		StartFireDelay();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}


void UCombatComponent::Server_SetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimingMoveSpeed : BaseMoveSpeed;
	}
}

