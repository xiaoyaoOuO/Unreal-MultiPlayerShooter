// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

UCLASS()
class BLAST_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void PlayDestroyedEffect();
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float Damage;

	UPROPERTY(EditAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent *BoxComponent;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& HitResult);

private:
	UPROPERTY(EditAnywhere,Category="Projectile")
	class UParticleSystem* Tracer;

	UPROPERTY(EditAnywhere,Category="Projectile")
	class UParticleSystem* DestroyedImpact;

	UPROPERTY(EditAnywhere,Category="Projectile")
	class USoundCue* DestroyedSound;

	UPROPERTY()
	UParticleSystemComponent* TracerParticleComponent;
};
