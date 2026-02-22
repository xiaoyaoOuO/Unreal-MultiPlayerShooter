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

	void JumpBuff(float JumpZVelocity = 1000.f, float BuffTime = 5.f);

	void ShieldBuff(float Shield, float BuffTime);

	UFUNCTION()
	void ResetSpeed();

	UFUNCTION()
	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetSpeed(float WalkSpeed, float CrouchWalkSpeed);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetJump(float JumpZVelocity);

protected:
	virtual void BeginPlay() override;
	void HealBuffUpdate(float DeltaTime);
	void ShieldBuffUpdate(float DeltaTime);
	void InitDefaultSpeed(float WalkSpeed, float CrouchWalkSpeed);
	void InitDefaultJump(float JumpZVelocity);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	//Health Buff
	bool  bIsHealthBuffActive;
	float HealthAmount;
	float HealthRate;

	//Speed Buff
	FTimerHandle SpeedBuffTimer;
	float OriginCrouchSpeed;
	float OriginWalkSpeed;

	//Jump Buff
	FTimerHandle JumpBuffTimer;
	float OriginJumpZVelocity;

	//Shield Buff
	bool  bIsShieldBuffActive;
	float ShieldAmount;
	float ShieldRate;
};
