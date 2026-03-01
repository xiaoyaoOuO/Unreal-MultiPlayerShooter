// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	//子弹移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileBulletMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed     = InitialSpeed;
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (AController* Controller = OwnerCharacter->Controller)
		{
			UGameplayStatics::ApplyDamage(OtherActor,Damage,Controller,this,UDamageType::StaticClass());
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);
	
	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this,PathParams,PredictResult);
}

void AProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if  WITH_EDITOR
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
#endif
	
}
