// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/BlasterType/TeamType.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"

UCLASS()
class BLAST_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlagZone();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* ZoneSphere;

	UPROPERTY(EditDefaultsOnly)
	ETeam TeamTag;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DecoratedMesh; //装饰用的静态网格组件，玩家无法与之交互
};
