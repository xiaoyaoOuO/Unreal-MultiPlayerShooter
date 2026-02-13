// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float Damage;

	UPROPERTY(EditAnywhere,Category="Weapon")
	UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere,Category=  "Weapon")
	UParticleSystem* BeamParticle;

public:
	virtual void Fire(const FVector& HitTarget) override;
	
};
