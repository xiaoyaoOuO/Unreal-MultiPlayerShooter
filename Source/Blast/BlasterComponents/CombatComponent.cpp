// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blast/Weapon/Weapon.h"
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
	UE_LOG(LogTemp,Warning,TEXT("TraceUnderCrosshairs"));
	FVector2D ViewPortSize;
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
		UE_LOG(LogTemp,Warning,TEXT("detecting crosshair"));
		FVector StartLocation = CrosshairWorldPosition;
		FVector EndLocation = StartLocation + CrosshairWorldDirection * TRACE_LENGTH;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			StartLocation,
			EndLocation,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndLocation;
		}
	}
}

void UCombatComponent::UpdateHUD(float deltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			FHUDPackage HUDPackage;
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
				InAirFactor = FMath::FInterpTo(InAirFactor,2.25f,deltaTime,2.25f);
			}else
			{
				InAirFactor = FMath::FInterpTo(InAirFactor,0.f,deltaTime,30.f);
			}
			HUDPackage.SpreadSize = WalkSpeedFactor + InAirFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
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

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
	this->HitTargetPoint = HitResult.ImpactPoint;
	UpdateHUD(DeltaTime);
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

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
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

