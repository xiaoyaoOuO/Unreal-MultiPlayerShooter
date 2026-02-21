// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "JumpPickUp.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AJumpPickUp : public APickUp
{
	GENERATED_BODY()
public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void Destroyed() override;
private:
	UPROPERTY(EditAnywhere)
	float BuffJumpZVelocity = 1000.f;

	UPROPERTY(EditAnywhere)
	float BuffTime = 10.f;
	
};
