// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	AActor* WeaponOwner =  GetOwner();
	if (WeaponOwner == nullptr || WeaponMesh == nullptr) return;
	AController* Controller = WeaponOwner->GetInstigatorController();

	if (UWorld* World = GetWorld())
	{
		if (const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash")))
		{
			FHitResult HitResult;
			FVector StartLocation = MuzzleSocket->GetSocketTransform(WeaponMesh).GetLocation();
			FVector EndLocation = StartLocation + (HitTarget - StartLocation) * 1.25f;
			FVector BeamLocation = EndLocation;
			World->LineTraceSingleByChannel(HitResult, MuzzleSocket->GetSocketTransform(WeaponMesh).GetLocation(), EndLocation, ECC_Visibility);
			if (HitResult.bBlockingHit)
			{
				BeamLocation = HitResult.ImpactPoint;
				if (AActor* HitActor = HitResult.GetActor())
				{
					if (HasAuthority() && Controller)
					{
						UGameplayStatics::ApplyDamage(HitActor,Damage,Controller,this,UDamageType::StaticClass());
					}
				}

				if (HitParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(this,HitParticle,HitResult.ImpactPoint,HitResult.ImpactNormal.Rotation());
				}
			}
			if (BeamParticle)
			{
				if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(this,BeamParticle,StartLocation))
				{
					Beam->SetVectorParameter("Target",BeamLocation);
				}
			}
		}
	}
}
