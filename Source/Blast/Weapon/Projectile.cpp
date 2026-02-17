// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Blast/Blast.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	//仅对可见的物体和墙壁等做碰撞检测
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			BoxComponent,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);
	}
	
}

void AProjectile::PlayDestroyedEffect()
{
	if (DestroyedImpact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),DestroyedImpact,GetActorTransform());
	}
	if (DestroyedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),DestroyedSound,GetActorLocation());
	}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	PlayDestroyedEffect();
}

void AProjectile::ExplodeDamage()
{
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	Destroy();
}

void AProjectile::BulletSpawnEffect()
{
	if (TrailNiagaraSystem)
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagaraSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition,
			false
		);
	}

	if (SpawnSound)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAttached(
			SpawnSound,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition,
			false,
			1,
			1,
			0,
			SpawnSoundAttenuation
		);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

