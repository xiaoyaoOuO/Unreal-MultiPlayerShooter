// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"
#include "Kismet/GameplayStatics.h"

ARocketProjectile::ARocketProjectile()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
/*
* UFUNCTION(BlueprintCallable)
static bool ApplyRadialDamageWithFalloff(
	const UObject* WorldContextObject, 
	float BaseDamage, 
	float MinimumDamage, 
	const FVector& Origin, 
	float DamageInnerRadius, 
	float DamageOuterRadius, 
	float DamageFalloff, 
	TSubclassOf<class UDamageType> DamageTypeClass, 
	const TArray<AActor*>& IgnoreActors, 
	AActor* DamageCauser = 0, 
	AController* InstigatedByController = 0, 
	ECollisionChannel DamagePreventionChannel = ECC_Visibility)
  (class UGameplayStatics 中
 */
	if (AActor* FireInstigator = GetInstigator())
	{
		if (AController* InstigatorController = FireInstigator->GetInstigatorController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				200.f,
				500.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				InstigatorController
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}
