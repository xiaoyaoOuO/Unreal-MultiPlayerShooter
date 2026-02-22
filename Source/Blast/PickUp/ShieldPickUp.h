// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "ShieldPickUp.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AShieldPickUp : public APickUp
{
	GENERATED_BODY()

public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	int32 ReplenishShield = 10;
	
	UPROPERTY(EditAnywhere)
	float ReplenishTime = 5.f;
};
