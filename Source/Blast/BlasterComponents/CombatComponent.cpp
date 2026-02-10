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

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr) return;
	CombatState = ECombatState::ECS_Reloading; //通过复制State令客户端播放装弹动画
	HandleReload();
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

void UCombatComponent::HandleReload()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	Character->PlayReloadMontage();
}

void UCombatComponent::OnReloadComplete()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateWeaponAmmo();
	}
	if (bFireButtonPressed)
	{
		Fire();
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
	if (GetOwner()->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character && EquippedWeapon)
	{
		//装备后直接在本地先修改state，先取消物理模拟，这样Attach就不会产生冲突
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket"))
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;    //关闭随移动转向
		Character->bUseControllerRotationYaw = true;
		// EquippedWeapon->UpdateAmmoAmountHUD();
		//拾取音效
		if (EquippedWeapon->EquippedSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(),
				EquippedWeapon->EquippedSound,
				Character->GetActorLocation()
			);
		}
	}
}

void UCombatComponent::OnRep_CarriedAmmoAmount()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			FHUDData HUDData;
			HUDData.CarriedAmmo = CarriedAmmoAmount;
			Controller->SetBlasterPlayerHUDData(EHT_CarriedAmmo, HUDData);
		}
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWT_AssaultRifle,InitialCarried_AR_Ammo);
}

int32 UCombatComponent::AmountToReload(const EWeaponType& WeaponType) const
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		int32 CarriedAmmo = CarriedAmmoMap[WeaponType];
		int32 MagEmptySpace = EquippedWeapon->Get_MagCapacity() - EquippedWeapon->Get_AmmoAmount();
		return FMath::Min(CarriedAmmo,MagEmptySpace);
	}
	return 0;	
}

void UCombatComponent::UpdateWeaponAmmo()
{
	if (EquippedWeapon == nullptr) return;
	EWeaponType EquippedWeaponType = EquippedWeapon->Get_WeaponType();
	if (CarriedAmmoMap.Contains(EquippedWeaponType))
	{
		int32 AmmoToAdd = AmountToReload(EquippedWeaponType);
		EquippedWeapon->AddAmmo(AmmoToAdd);
		//更新携带弹药量
		CarriedAmmoMap[EquippedWeaponType] -= AmmoToAdd;
		this->CarriedAmmoAmount = CarriedAmmoMap[EquippedWeaponType];
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			FHUDData HUDData;
			HUDData.CarriedAmmo = CarriedAmmoAmount;
			Controller->SetBlasterPlayerHUDData(EHT_CarriedAmmo, HUDData);
		}
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
	if (EquippedWeapon->Get_AmmoAmount() <= 0 && CarriedAmmoAmount > 0)
	{
		Reload();
	}
}

bool UCombatComponent::CanFire() const
{
	if (EquippedWeapon == nullptr) return false;
	return EquippedWeapon->Get_AmmoAmount() > 0 && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
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
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmoAmount,COND_OwnerOnly);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Character == nullptr || Weapon ==nullptr) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket"))
	{
		HandSocket->AttachActor(Weapon, Character->GetMesh());
	}
	Weapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;    //关闭随移动转向
	Character->bUseControllerRotationYaw = true;
	//更新携带弹药量
	EWeaponType EquippedWeaponType = EquippedWeapon->Get_WeaponType();
	if (CarriedAmmoMap.Contains(EquippedWeaponType))
	{
		CarriedAmmoAmount = CarriedAmmoMap[EquippedWeaponType];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		FHUDData HUDData;
		HUDData.CarriedAmmo = CarriedAmmoAmount;
		Controller->SetBlasterPlayerHUDData(EHT_CarriedAmmo, HUDData);
	}
	if (EquippedWeapon->EquippedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			EquippedWeapon->EquippedSound,
			Character->GetActorLocation()
		);
	}
	if (EquippedWeapon->Get_AmmoAmount()<=0 && CarriedAmmoAmount > 0)
	{
		Reload();
	}
	EquippedWeapon->UpdateAmmoAmountHUD();
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
	if (CanFire())
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

void UCombatComponent::Reload()
{
	if (CarriedAmmoAmount > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
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

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
	default:
		break;
	}
}
