// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "PickUp.h"
#include "HealthPickUp.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()

public:
	AHealthPickUp();

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


protected:
	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* HealEffect;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* HealthComponent;

	UPROPERTY(EditAnywhere)
	int32 HealAmount = 100;
	
	UPROPERTY(EditAnywhere)
	float HealTime = 5.f;
};
