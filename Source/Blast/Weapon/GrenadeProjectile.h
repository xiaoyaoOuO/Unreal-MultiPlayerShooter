// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "GrenadeProjectile.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API AGrenadeProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	AGrenadeProjectile();

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void Destroyed() override;
	
private:
	UPROPERTY(EditAnywhere,Category=Projectile)
	UStaticMeshComponent* GrenadeMesh;

	UFUNCTION()
	void OnBounce(const FHitResult& Hit,const FVector& NormalImpulse);

protected:
	virtual void ExplodeDamage() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;

	FTimerHandle LifeTimer;

	UPROPERTY(EditAnywhere,Category=Projectile)
	float LifeTime;

	UPROPERTY(EditAnywhere,Category=Projectile)
	float DamageInnerRadius;

	UPROPERTY(EditAnywhere,Category=Projectile)
	float DamageOuterRadius;
};
