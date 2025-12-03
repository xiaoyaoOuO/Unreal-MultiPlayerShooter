// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//Spawn Projectile
	const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleSocket)
	{
		//获取生成子弹的位置（武器mesh存在一个枪口的槽位）
		FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
		FVector ToTarget = HitTarget - MuzzleTransform.GetLocation();
		FRotator Rotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass, // 子弹的蓝图类
					MuzzleTransform.GetLocation(),
					Rotation,
					SpawnParameters
				);
			}
		}
	}
}
