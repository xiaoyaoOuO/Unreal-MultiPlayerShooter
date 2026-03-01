// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// //客户端不需要生成子弹，服务器会生成并replicate
	// if (!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	if (InstigatorPawn == nullptr || World == nullptr) return;
	//Spawn Projectile
	if (const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash")))
	{
		//获取生成子弹的位置（武器mesh存在一个枪口的槽位）
		FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
		FVector ToTarget = HitTarget - MuzzleTransform.GetLocation();
		FRotator Rotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority())
			{
				if (InstigatorPawn->IsLocallyControlled()) // 服务端且本地控制，
				{
					if (ProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,MuzzleTransform.GetLocation(),Rotation,SpawnParameters);
						SpawnedProjectile->bUseServerRewind = false;
					}
				}else // 服务端但非本地控制，说明是其他玩家的武器在服务端生成子弹
				{
					if (ServerProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerProjectileClass,MuzzleTransform.GetLocation(),Rotation,SpawnParameters);
						SpawnedProjectile->bUseServerRewind = false;
					}
				}
			}else
			{
				if (InstigatorPawn->IsLocallyControlled()) // 客户端且本地控制,需要生成一个服务端版本的子弹来进行服务器回放
				{
					if (ServerProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerProjectileClass,MuzzleTransform.GetLocation(),Rotation,SpawnParameters);
						SpawnedProjectile->bUseServerRewind = true;
						SpawnedProjectile->TraceStart = MuzzleTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					}
				}else    // 在客户端上，其他玩家的复制
				{
					if (ProjectileClass)
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,MuzzleTransform.GetLocation(),Rotation,SpawnParameters);
						SpawnedProjectile->bUseServerRewind = false;
					}
				}
			}
		}else // 不使用服务器回放，只在服务器上生成子弹并复制到客户端
		{
			if (InstigatorPawn->HasAuthority() && ProjectileClass)
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,MuzzleTransform.GetLocation(),Rotation,SpawnParameters);
				SpawnedProjectile->bUseServerRewind = false;
			}			
		}
	}
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
}
