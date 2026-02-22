// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpSpawnPoint.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	SpawnPickUpTimerFinished();
}

void APickUpSpawnPoint::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			int PickUpClassNums = PickUpClasses.Num();
			if (PickUpClassNums > 0)
			{
				int SpawnSelection = FMath::RandRange(0,PickUpClassNums-1);
				SpawnPickUp = World->SpawnActor<APickUp>(PickUpClasses[SpawnSelection],GetActorTransform());
				if (SpawnPickUp)
				{
					SpawnPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::OnPickUpDestroyed);
				}
			}
		}
	}
}

void APickUpSpawnPoint::OnPickUpDestroyed(AActor* DestroyedActor)
{
	if (UWorld* World = GetWorld())
	{
		float SpawnTime = FMath::RandRange(SpawnTimeMin,SpawnTimeMax);
		World->GetTimerManager().SetTimer(SpawnTimerHandle,this,&APickUpSpawnPoint::SpawnPickUpTimerFinished, SpawnTime);
	}
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

