// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeProjectile.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AGrenadeProjectile::AGrenadeProjectile()
{
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>("GrenadeMesh");
	GrenadeMesh->SetupAttachment(RootComponent);
	GrenadeMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("GrenadeProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AGrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
}

void AGrenadeProjectile::OnBounce(const FHitResult& Hit,const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,BounceSound,GetActorLocation());
	}
}

void AGrenadeProjectile::ExplodeDamage()
{
	AActor* FireInstigator = GetInstigator();
	if (FireInstigator && HasAuthority())
	{
		if (AController* InstigatorController = FireInstigator->GetInstigatorController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				DamageInnerRadius,
				DamageOuterRadius,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				InstigatorController
			);
		}
	}
}

void AGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	//Delay一段时间再销毁，保留一段时间特效
	GetWorldTimerManager().SetTimer(
		LifeTimer,
		[this]()
		{
			Destroy();
		},
		LifeTime,
		false
	);

	BulletSpawnEffect();
	
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this,&AGrenadeProjectile::OnBounce);
}

void AGrenadeProjectile::Destroyed()
{
	//ApplyDamage
	ExplodeDamage();
	Super::Destroyed();
}


