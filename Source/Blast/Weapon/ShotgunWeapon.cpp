// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunWeapon.h"

#include "Blast/Blast.h"
#include "Blast/BlasterComponents/LagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

void AShotgunWeapon::ShotgunFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	/*
	 * 武器动画以及子弹壳的生成
	 */
	UWorld* World = GetWorld();
	FHitResult HitResult;
	if (!World) return;
	if (FireAnimationAsset)
	{
		WeaponMesh->PlayAnimation(FireAnimationAsset,false);
	}
	if (BulletShell)
	{
		if (const USkeletalMeshSocket* AmmoEject = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			//获取生成子弹的位置（武器mesh存在一个枪口的槽位）
			FTransform MuzzleTransform = AmmoEject->GetSocketTransform(WeaponMesh);
			FRotator Rotation = MuzzleTransform.GetRotation().Rotator();
			World->SpawnActor<ACasting>(
				BulletShell, // 子弹的蓝图类
				MuzzleTransform.GetLocation(),
				Rotation
			);
		}
	}
	SpendAmmo();

	/**
	 * 霰弹枪的伤害计算：每发射出一颗子弹就进行一次LineTrace，如果击中角色就记录下来，最后根据每个角色被击中的次数来计算总伤害
	 */
	TMap<ABlasterCharacter*, int32> HitMap;
	FVector StartLocation = AmmoSpawnLocation();
	for (const FVector_NetQuantize& HitTarget : HitTargets)
	{
		FVector EndLocation = StartLocation + (HitTarget - StartLocation) * 1.25f;
		World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_HitBox);
		FVector BeamLocation = EndLocation;
		if (HitResult.bBlockingHit)
		{
			BeamLocation = HitResult.ImpactPoint;
			if (ABlasterCharacter* HitActor = Cast<ABlasterCharacter>(HitResult.GetActor()))
			{
				if (HitMap.Contains(HitActor))
				{
					HitMap[HitActor]++;
				}else
				{
					HitMap.Add(HitActor,1);
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
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter == nullptr) return;
	if (HasAuthority() && OwnerCharacter->IsLocallyControlled())
	{
		for (const auto &HitPair : HitMap)
		{
			ABlasterCharacter* HitActor = HitPair.Key;
			int32 PelletHitCount = HitPair.Value;
			if (HitActor && Controller)
			{
				float TotalDamage = Damage * PelletHitCount;
				UGameplayStatics::ApplyDamage(HitActor,TotalDamage,Controller,this,UDamageType::StaticClass());
			}
		}
	}
	if (!HasAuthority() && bUseServerSideRewind && !HitMap.IsEmpty() && OwnerCharacter->IsLocallyControlled())
	{
		TArray<ABlasterCharacter*> HitActors;
		for (auto HitPair : HitMap)
		{
			HitActors.Add(HitPair.Key);
		}
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		ULagCompensationComponent* OwnerLagCompensation = OwnerCharacter->GetLagCompensationComponent();
		if (OwnerLagCompensation && OwnerController)
		{
			UE_LOG(LogTemp,Warning,TEXT("Client_ShotgunScoreRequest"));
			OwnerLagCompensation->Server_ShotgunScoreRequest(StartLocation,HitTargets,HitActors,OwnerController,this,OwnerController->GetServerTime() - OwnerController->SoloTripTime);
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

void AShotgunWeapon::Client_UpdateAddAmmo(int32 Amount)
{
	Super::Client_UpdateAddAmmo(Amount);

	if (IsFull())
	{
		if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwner()))
		{
			Character->JumpToShotgunEnd();
		}
	}
}


void AShotgunWeapon::GetScatterEndLocations(TArray<FVector_NetQuantize>& HitTargets, const FVector& HitTarget)
{
	FVector StartLocation = AmmoSpawnLocation();
	FVector ShotDirection = (HitTarget - StartLocation).GetSafeNormal();
	FVector SphereCenter = StartLocation + ShotDirection * SphereDistance;
	for (int i=0;i<NumberOfPellets;i++)
	{
		//在散射范围内随机一个点
		FVector RandomScaterVector = UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(0.f, ScatterRadius);
		FVector RandomPoint = SphereCenter + RandomScaterVector;

		HitTargets.Add(RandomPoint);
	}
}
