// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "Blast/HUD/BlasterHUD.h"
#include "Blast/Weapon/Projectile.h"
#include "Blast/Weapon/WeaponType.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLAST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void AttachWeaponToRightHand(AWeapon* Weapon);
	friend class ABlasterCharacter;
	void EquipWeapon(AWeapon* Weapon);
	void SetAiming(bool bIsAiming);
	void Fire();
	void FireButtonPressed(bool bPressed);
	void Reload();
	void ThrowGrenade();
	void PickUpAmmo(EWeaponType WeaponType,int32 AmmoAmount);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(FVector_NetQuantize HitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize HitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(Server, Reliable)
	void Server_GrenadeToss();

	UFUNCTION(Server, Reliable)
	void Server_SpawnGrenade(const FVector_NetQuantize HitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void UpdateHUD(float DeltaTime);

	void InterpFOV(float DeltaTime);

	void HandleReload();

	void UpdateGrenadeHUD();

	UFUNCTION(BlueprintCallable)
	void OnReloadComplete();
	void UpdateCarriedAmmoHUD();

	UFUNCTION(BlueprintCallable)
	void OnAddShotgunAmmo();

	UFUNCTION(BlueprintCallable)
	void OnGrenadeTossFinished();

	UFUNCTION(BlueprintCallable)
	void OnGrenadeLaunch();
	
	FORCEINLINE ECombatState Get_CombatState() const {return CombatState;}
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmoAmount();
	
	UFUNCTION()
	void OnRep_CombatState();
	
	UFUNCTION()
	void OnRep_GrenadeAmount();

	void InitializeCarriedAmmo();
	
	int32 AmountToReload(const EWeaponType& WeaponType) const;

	void UpdateWeaponAmmo();

	void PutWeaponToBack();

public:
	class ABlasterCharacter* Character;
	class ABlasterPlayerController* Controller;
	class ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimingMoveSpeed;

	bool bFireButtonPressed;

	FVector HitTargetPoint;

	FHUDPackage HUDPackage;

	//瞄准调整FOV
	float CurrentFOV;
	float DefaultFOV;
	float ZoomFOV = 30.f;
	float ZoomSpeed = 10.f;

private:
	//准心扩散因子
	float WalkSpeedFactor;
	float InAirFactor;
	float AimFactor;
	float ShootingFactor;

	/*
	 * 开火属性
	*/
	bool bCanFire = true;
	float FireDelay;
	FTimerHandle FireDelayTimer;
	void StartFireDelay();
	void FireTimerFinish();
	bool CanFire() const;

	//对于当前武器，玩家所携带的子弹数量
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmoAmount)
	int32 CarriedAmmoAmount;

	UPROPERTY(ReplicatedUsing=OnRep_GrenadeAmount)
	int32 CarriedGrenadeAmount;

	//初始携带弹药
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_AR_Ammo;				//步枪
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_Rocket_Ammo;			//火箭筒
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_Pistol_Ammo;			//手枪
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_SMG_Ammo;				//冲锋枪
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_Shotgun_Ammo;			//霰弹枪
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_Sniper_Ammo;			//狙击步枪
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_GrenadeLauncher_Ammo;	//榴弹发射器
	UPROPERTY(EditAnywhere,Category="Combat")
	int32 InitialCarried_ThrownGrenade;			//手雷
	TMap<EWeaponType,int32> CarriedAmmoMap;
	
	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState;

	UPROPERTY(EditAnywhere,Category="Combat")
	TSubclassOf<AProjectile> ThrownGrenade;
};
