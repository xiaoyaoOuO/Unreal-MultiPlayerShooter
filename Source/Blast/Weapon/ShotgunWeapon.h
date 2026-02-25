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
	void ShotgunFire(const TArray<FVector_NetQuantize>& HitTarget);
	void GetScatterEndLocations(TArray<FVector_NetQuantize>& HitTargets,const FVector& HitTarget);

protected:
	virtual FVector TraceEndWithScatter(const FVector& StartLocation,const FVector& HitTarget) override;
	virtual void Client_UpdateAddAmmo(int32 Amount) override;

private:
	UPROPERTY(EditAnywhere,Category="Weapon")
	int32 NumberOfPellets;
};
