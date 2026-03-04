// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Blast/BlasterComponents/LagCompensationComponent.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
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
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (HitCharacter && OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->GetController());
		if (OwnerController == nullptr) return;

		if (OwnerCharacter->HasAuthority() && !bUseServerRewind)
		{
			float DamageToApply = Damage;
			if (HitResult.BoneName == FName("head"))
			{
				DamageToApply *= 1.5f;
			}
			UGameplayStatics::ApplyDamage(HitCharacter, DamageToApply, OwnerController, this, UDamageType::StaticClass());
		}
		if (!OwnerCharacter->HasAuthority() && bUseServerRewind) //客户端需要向服务器发送请求，让服务器根据客户端传来的TraceStart和HitLocation进行服务器回放，确认命中有效性
		{
			if (ULagCompensationComponent* LagCompensationComponent = OwnerCharacter->GetLagCompensationComponent())
			{
				LagCompensationComponent->Server_ProjectileScoreRequest(TraceStart,InitialVelocity,HitCharacter,OwnerController,
					OwnerController->GetServerTime()-OwnerController->SoloTripTime,Damage);
			}
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
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);
	
	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this,PathParams,PredictResult);

	if (!HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this,&AProjectileBullet::OnHit);
	}
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif
