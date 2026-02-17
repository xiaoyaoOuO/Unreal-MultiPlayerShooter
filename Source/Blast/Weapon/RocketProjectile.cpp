// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"

#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ARocketProjectile::ARocketProjectile()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	//子弹移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = 5000.0f;
	ProjectileMovementComponent->MaxSpeed     = 5000.0f;
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("ARocketProjectile::OnHit, hit self, ignore"));
		return;
	}
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
	ExplodeDamage();

	/*不调用父类OnHit，单独处理OnHit情况*/
	PlayDestroyedEffect();

	//Delay一段时间再销毁，保留一段时间特效
	GetWorldTimerManager().SetTimer(
		OnHitTimer,
		[this]()
		{
			Destroy();
		},
		DestroyDelay,
		false
	);

	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (this->ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
	}
	if (TrailComponent)
	{
		TrailComponent->Deactivate();
	}
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
}

void ARocketProjectile::BeginPlay()
{
	Super::BeginPlay();

	//Destroyed啥也没干，需要客户端自己处理OnHit
	if (!HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this,&ARocketProjectile::OnHit);
	}

	BulletSpawnEffect();
}

void ARocketProjectile::Destroyed()
{
	//什么也不干
}

void ARocketProjectile::ExplodeDamage()
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
}
