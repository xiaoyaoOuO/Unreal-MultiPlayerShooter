// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	virtual FVector TraceEndWithScatter(const FVector& StartLocation,const FVector& HitTarget) override;
	virtual void WeaponHit(FHitResult& HitResult, const UWorld* World, const FVector& StartLocation, const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere,Category="Weapon")
	int32 NumberOfPellets;
};
