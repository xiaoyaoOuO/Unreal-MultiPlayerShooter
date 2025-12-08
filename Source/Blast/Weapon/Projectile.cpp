// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

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

	//子弹移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 15000.0f;
	ProjectileMovementComponent->MaxSpeed     = 15000.0f;

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

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (DestroyedImpact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),DestroyedImpact,GetActorTransform());
	}
	if (DestroyedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),DestroyedSound,GetActorLocation());
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	Destroy();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

