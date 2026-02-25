// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casting.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Blast/Character/BlasterCharacter.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Secondary UMETA(DisplayName = "Secondary"), //副武器

	EWS_MAX UMETA(DisplayName = "Default MAX")
};

UENUM()
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan"),
	EFT_Projectile UMETA(DisplayName = "Projectile"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLAST_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnRep_Owner() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int OtherBodyIndex
	);

	UFUNCTION(Client, Reliable)
	virtual void Client_UpdateAmmo(int32 Amount);

	UFUNCTION(Client, Reliable)
	virtual void Client_UpdateAddAmmo(int32 Amount);

	int32 AmmoSequence = 0;	//用于客户端预测，防止网络延迟导致的UI更新错误

	void SpendAmmo();

protected:
	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;
	UPROPERTY()
	ABlasterCharacter* BlasterPlayerCharacter;

	UPROPERTY(EditAnywhere,Category = "Weapon")
	EWeaponType WeaponType;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void AddAmmo(int32 Amount);

	FORCEINLINE bool IsFull(){return AmmoAmount == MagCapacity;}

	void ShowPickUpWidget(bool bShowPickupWidget) const;
	void OnDropped();
	void OnEquipped();
	void OnSecondary();
	void OnStateSet();

	void SetWeaponState(EWeaponState State);

	UFUNCTION()
	void OnRep_WeaponState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FVector AmmoSpawnLocation() const;

	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();

	USkeletalMeshComponent* Get_WeaponMesh() const;

	void UpdateAmmoAmountHUD();

	FORCEINLINE int32 Get_AmmoAmount() const {return AmmoAmount;}
	FORCEINLINE EWeaponType Get_WeaponType() const {return WeaponType;}
	FORCEINLINE int32 Get_MagCapacity() const {return MagCapacity;}
	FORCEINLINE EFireType Get_FireType() const {return FireType;}


protected:
	void SetDepthRender(bool bEnable);
	
public:
	UPROPERTY(VisibleAnywhere,Category =  "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere,Category = "Weapon")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_WeaponState,Category = "Weapon")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere,Category = "Weapon")
	UWidgetComponent* PickUpWidget;
	
	UPROPERTY(EditAnywhere,Category = "Weapon")
	UAnimationAsset* FireAnimationAsset;
	
	UPROPERTY(EditAnywhere,Category = "Weapon") 
	TSubclassOf<ACasting> BulletShell;

	UPROPERTY(EditAnywhere,Category = "Weapon")
	USoundCue* EquippedSound;

	/*
	 武器准心
	 */
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairCenter;
	
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairLeft;
	
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairRight;
	
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairTop;
	
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairBottom;

	/*
	 *武器瞄准时的FOV
	 */
	UPROPERTY(EditAnywhere,Category="Zoom")
	float ZoomFOV = 30.f;
	UPROPERTY(EditAnywhere,Category="Zoom")
	float ZoomSpeed = 10.f;

	/*
	 * 开火属性
	 */
	UPROPERTY(EditAnywhere,Category="Weapon")
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere,Category="Weapon")
	bool bAutomaticFire = true;
	UPROPERTY(EditAnywhere,Category="Weapon")
	EFireType FireType;


	//初始武器需要在角色死亡时销毁
	bool bShouldDestroy = false;

private:
	UPROPERTY(EditAnywhere,Category="Weapon")
	int32 AmmoAmount;

	UPROPERTY(EditAnywhere,Replicated,Category="Weapon")
	int32 MagCapacity;
};
