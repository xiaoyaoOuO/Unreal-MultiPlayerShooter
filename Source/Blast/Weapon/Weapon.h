// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casting.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default MAX")
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
	virtual void OnRep_AmmoAmount();

	FORCEINLINE int32 Get_AmmoAmount() const {return AmmoAmount;};

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

	void SpendAmmo();

protected:
	ABlasterPlayerController* BlasterPlayerController;
	ABlasterCharacter* BlasterPlayerCharacter;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShowPickUpWidget(bool bShowPickupWidget) const;

	void SetWeaponState(EWeaponState State);

	UFUNCTION()
	void OnRep_WeaponState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Fire(const FVector& HitTarget);
	virtual void Dropped();

	USkeletalMeshComponent* Get_WeaponMesh() const;

	void UpdateAmmoAmountHUD();
	
public:
	UPROPERTY(VisibleAnywhere,Category =  "Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere,Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_WeaponState,Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere,Category = "Weapon Properties")
	class UWidgetComponent* PickUpWidget;
	
	UPROPERTY(EditAnywhere,Category = "Weapon Properties")
	class UAnimationAsset* FireAnimationAsset;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasting> BulletShell;

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
	UPROPERTY(EditAnywhere,Category="Weapon Attributes")
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere,Category="Weapon Attributes")
	bool bAutomaticFire = true;

private:
	UPROPERTY(EditAnywhere,ReplicatedUsing = OnRep_AmmoAmount,Category="Weapon Attributes")
	int32 AmmoAmount;

	UPROPERTY(EditAnywhere,Replicated,Category="Weapon Attributes")
	int32 MagCapacity;
};
