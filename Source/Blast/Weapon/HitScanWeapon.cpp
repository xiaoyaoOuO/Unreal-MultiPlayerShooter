// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blast/BlasterComponents/LagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::FireEffect(const FVector& StartLocation,const FVector& BeamLocation)
{
	if (BeamParticle)
	{
		if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(this,BeamParticle,StartLocation))
		{
			Beam->SetVectorParameter("Target",BeamLocation);
		}
	}
	if (FireParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this,FireParticle,StartLocation);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,FireSound,StartLocation);
	}
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
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

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& StartLocation, const FVector& HitTarget)
{
	if (!bUseScatter) return HitTarget;
	FVector ShotDirection = (HitTarget - StartLocation).GetSafeNormal();
	FVector SphereCenter = StartLocation + ShotDirection * SphereDistance;

	// DrawDebugSphere(GetWorld(), SphereCenter, ScatterRadius, 12, FColor::Red, false, 2.f);
	//在散射范围内随机一个点
	FVector RandomScaterVector = UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(0.f, ScatterRadius);
	FVector RandomPoint = SphereCenter + RandomScaterVector;
	// DrawDebugPoint(GetWorld(), RandomPoint, 1.f, FColor::Green, false, 2.f);
	return RandomPoint;
}

void AHitScanWeapon::WeaponHit(FHitResult& HitResult,const UWorld* World,const FVector& StartLocation,const FVector& HitTarget)
{
	FVector EndLocation = StartLocation + (HitTarget - StartLocation) * 1.25f;
	World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);
	FVector BeamLocation = EndLocation;
	if (HitResult.bBlockingHit)
	{
		BeamLocation = HitResult.ImpactPoint;
		AController* Controller = GetOwner() ? GetOwner()->GetInstigatorController() : nullptr;
		ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
		if (HitCharacter && Controller)
		{
			if (HasAuthority() && !bUseServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(HitCharacter,Damage,Controller,this,UDamageType::StaticClass());
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
				if (OwnerCharacter)
				{
					ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
					ULagCompensationComponent* OwnerLagCompensation = OwnerCharacter->GetLagCompensationComponent();
					if (OwnerLagCompensation && OwnerController)
					{
						float ServerHitTime = OwnerController->GetServerTime() - OwnerController->SoloTripTime;
						OwnerLagCompensation->Server_ScoreRequest(StartLocation,EndLocation,HitCharacter,OwnerController,this,ServerHitTime);
					}
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
