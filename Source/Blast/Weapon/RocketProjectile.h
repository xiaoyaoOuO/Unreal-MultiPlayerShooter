// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "RocketProjectile.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ARocketProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	ARocketProjectile();
private:
	UPROPERTY(EditAnywhere,Category="Projectile")
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere,Category="Projectile")
	USoundCue* RocketSound;

	UPROPERTY(EditAnywhere,Category="Projectile")
	USoundAttenuation* RocketSoundAttenuation;

	UPROPERTY()
	UAudioComponent* AudioComponent;
	
	UPROPERTY(EditAnywhere,Category="Projectile")
	UNiagaraSystem* RocketTrailNiagaraSystem;

	UPROPERTY()
	UNiagaraComponent* RocketTrailComponent;


	/*碰撞后不马上销毁，保留特效一段时间*/
	FTimerHandle OnHitTimer;
	float DestroyDelay = 3.f;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override final;
};
