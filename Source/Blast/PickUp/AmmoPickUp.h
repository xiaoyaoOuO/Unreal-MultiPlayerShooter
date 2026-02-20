// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "Blast/Weapon/WeaponType.h"
#include "AmmoPickUp.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AAmmoPickUp : public APickUp
{
	GENERATED_BODY()

public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditAnywhere, Category="PickUp")
	EWeaponType AmmoType;

	UPROPERTY(EditAnywhere,Category="PickUp")
	int32 AmmoAmount;
};
