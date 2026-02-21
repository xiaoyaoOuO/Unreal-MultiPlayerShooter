// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLAST_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;

	void HealthBuff(float Health, float BuffTime);

	void SpeedBuff(float WalkSpeed, float CrouchWalkSpeed ,float BuffTime);

	void ResetSpeed();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetSpeed(float WalkSpeed, float CrouchWalkSpeed);

protected:
	virtual void BeginPlay() override;
	void HealBuffUpdate(float DeltaTime);
	void InitDefaultSpeed(float WalkSpeed, float CrouchWalkSpeed);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	//Health Buff
	bool  bIsHealthBuffActive;
	float HealthAmount;
	float HealthTime;
	float HealthRate;

	//Speed Buff
	FTimerHandle SpeedBuffTimer;
	float OriginCrouchSpeed;
	float OriginWalkSpeed;
};
