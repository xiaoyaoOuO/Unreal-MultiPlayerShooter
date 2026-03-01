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

protected:
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float Damage;

	UPROPERTY(EditAnywhere,Category="Weapon")
	UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere,Category=  "Weapon")
	UParticleSystem* BeamParticle;

	UPROPERTY(EditAnywhere,Category=  "Weapon")
	UParticleSystem* FireParticle;

	UPROPERTY(EditAnywhere,Category=  "Weapon")
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere,Category="Weapon")
	float ScatterRadius = 75.f;

	UPROPERTY(EditAnywhere,Category="Weapon")
	float SphereDistance;
	
	UPROPERTY(EditAnywhere,Category="Weapon")
	bool bUseScatter = false;
public:
	void FireEffect(const FVector& StartLocation,const FVector& BeamLocation);
	virtual void Fire(const FVector& HitTarget) override;
	virtual FVector TraceEndWithScatter(const FVector& StartLocation,const FVector& HitTarget);
	virtual int GetDamage() override {return Damage;}

protected:
	virtual void WeaponHit(FHitResult& HitResult,const UWorld* World, const FVector& StartLocation ,const FVector& HitTarget);
};
