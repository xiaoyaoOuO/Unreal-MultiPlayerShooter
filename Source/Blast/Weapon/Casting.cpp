// Fill out your copyright notice in the Description page of Project Settings.


#include "Casting.h"

#include "Kismet/GameplayStatics.h"


ACasting::ACasting()
{
	PrimaryActorTick.bCanEverTick = false;

	CastingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CastingMesh"));
	CastingMesh->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	CastingMesh->SetEnableGravity(true);
	CastingMesh->SetSimulatePhysics(true);
	CastingMesh->SetNotifyRigidBodyCollision(true);
	Speed = 10.f;
	SetRootComponent(CastingMesh);
}

void ACasting::BeginPlay()
{
	Super::BeginPlay();

	CastingMesh->AddImpulse(GetActorForwardVector()*Speed);
	CastingMesh->OnComponentHit.AddDynamic(this,&ACasting::OnHit);
}

void ACasting::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	if (CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),CastSound,GetActorLocation());
	}
	Destroy();
}
