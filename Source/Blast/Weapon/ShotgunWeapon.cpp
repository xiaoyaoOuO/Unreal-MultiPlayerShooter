// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

void AShotgunWeapon::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	AActor* WeaponOwner =  GetOwner();
	if (WeaponOwner == nullptr || WeaponMesh == nullptr) return;

	if (UWorld* World = GetWorld())
	{
		if (const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash")))
		{
			FHitResult HitResult;
			FVector StartLocation = MuzzleSocket->GetSocketTransform(WeaponMesh).GetLocation();
			WeaponHit(HitResult,World,StartLocation,HitTarget);
		}
	}
	
}

FVector AShotgunWeapon::TraceEndWithScatter(const FVector& StartLocation, const FVector& HitTarget)
{
	FVector ShotDirection = (HitTarget - StartLocation).GetSafeNormal();
	FVector SphereCenter = StartLocation + ShotDirection * SphereDistance;

	DrawDebugSphere(GetWorld(), SphereCenter, ScatterRadius, 12, FColor::Red, false, 2.f);
	//在散射范围内随机一个点
	FVector RandomScaterVector = UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(0.f, ScatterRadius);
	FVector RandomPoint = SphereCenter + RandomScaterVector;
	DrawDebugPoint(GetWorld(), RandomPoint, 1.f, FColor::Green, false, 2.f);
	return RandomPoint;
}

void AShotgunWeapon::WeaponHit(FHitResult& HitResult, const UWorld* World,
	const FVector& StartLocation, const FVector& HitTarget)
{
	TMap<ABlasterCharacter*, int32> HitMap;
	for (int i=0;i<NumberOfPellets;i++)
	{
		FVector EndLocation = StartLocation + (TraceEndWithScatter(StartLocation,HitTarget) - StartLocation) * 1.25f;
		World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);
		FVector BeamLocation = EndLocation;
		if (HitResult.bBlockingHit)
		{
			BeamLocation = HitResult.ImpactPoint;
			if (ABlasterCharacter* HitActor = Cast<ABlasterCharacter>(HitResult.GetActor()))
			{
				if (HasAuthority())
				{
					if (HitMap.Contains(HitActor))
					{
						HitMap[HitActor]++;
					}else
					{
						HitMap.Add(HitActor,1);
					}
				}
			}
			if (HitParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(this,HitParticle,HitResult.ImpactPoint,HitResult.ImpactNormal.Rotation());
			}
		}
		FireEffect(StartLocation, BeamLocation);
	}
	AController* Controller = GetOwner() ? GetOwner()->GetInstigatorController() : nullptr;
	for (const auto &HitPair : HitMap)
	{
		ABlasterCharacter* HitActor = HitPair.Key;
		int32 PelletHitCount = HitPair.Value;
		if (HitActor && HasAuthority() && Controller)
		{
			float TotalDamage = Damage * PelletHitCount;
			UGameplayStatics::ApplyDamage(HitActor,TotalDamage,Controller,this,UDamageType::StaticClass());
		}
	}
}

void AShotgunWeapon::OnRep_AmmoAmount()
{
	Super::OnRep_AmmoAmount();

	if (IsFull())
	{
		if (BlasterPlayerCharacter)
		{
			BlasterPlayerCharacter->JumpToShotgunEnd();
		}
	}
}
