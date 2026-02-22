// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPoint.generated.h"

UCLASS()
class BLAST_API APickUpSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUpSpawnPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickUp>> PickUpClasses;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMin = 5.f;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMax = 10.f;

	FTimerHandle SpawnTimerHandle;

	UPROPERTY()
	APickUp* SpawnPickUp;

	UFUNCTION()
	void SpawnPickUpTimerFinished();

	//重新生成一个新的拾取物
	UFUNCTION()
	void OnPickUpDestroyed(AActor* DestroyedActor);
public:	
	virtual void Tick(float DeltaTime) override;

};
