// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "SpeedPickUp.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ASpeedPickUp : public APickUp
{
	GENERATED_BODY()

public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditAnywhere)
	float BuffWalkSpeed = 1200.f;

	UPROPERTY(EditAnywhere)
	float BuffCrouchWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere)
	float BuffTime = 5.f;
};
