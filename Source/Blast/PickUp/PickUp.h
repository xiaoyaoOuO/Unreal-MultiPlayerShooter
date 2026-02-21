// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/BlasterComponents/BuffComponent.h"
#include "PickUp.generated.h"

UCLASS()
class BLAST_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUp();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

private:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickUpMesh;

	UPROPERTY(EditAnywhere,Category="PickUp")
	USoundCue* PickUpSound;

protected:
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickUpEffect;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickUpNiagaraComponent;
};
