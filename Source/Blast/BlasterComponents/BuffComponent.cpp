// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blast/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::HealthBuff(float Health, float BuffTime)
{
	if (bIsHealthBuffActive) return;
	HealthAmount = Health;
	HealthTime = BuffTime;
	bIsHealthBuffActive = true;
	HealthRate = HealthAmount / HealthTime;
}

void UBuffComponent::SpeedBuff(float WalkSpeed, float CrouchWalkSpeed, float BuffTime)
{
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			OriginWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
			OriginCrouchSpeed = CharacterMovementComponent->MaxWalkSpeedCrouched;

			CharacterMovementComponent->MaxWalkSpeed = WalkSpeed;
			CharacterMovementComponent->MaxWalkSpeedCrouched = CrouchWalkSpeed;
			MulticastSetSpeed(WalkSpeed, CrouchWalkSpeed);
		}
	}
	GetWorld()->GetTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, BuffTime);
}

void UBuffComponent::ResetSpeed()
{
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->MaxWalkSpeed = OriginWalkSpeed;
			CharacterMovementComponent->MaxWalkSpeedCrouched = OriginCrouchSpeed;
			MulticastSetSpeed(OriginWalkSpeed, OriginCrouchSpeed);
		}
	}
}

void UBuffComponent::MulticastSetSpeed_Implementation(float WalkSpeed, float CrouchWalkSpeed)
{
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->MaxWalkSpeed = WalkSpeed;
			CharacterMovementComponent->MaxWalkSpeedCrouched = CrouchWalkSpeed;
		}
	}
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UBuffComponent::HealBuffUpdate(float DeltaTime)
{
	if (bIsHealthBuffActive)
	{
		float ThisFrameShouldHeal = HealthRate * DeltaTime;
		if (Character)
		{
			HealthAmount -= ThisFrameShouldHeal;
			
			float CharacterCurrentHealth = Character->Get_CurrentHealth();
			CharacterCurrentHealth = FMath::Clamp(CharacterCurrentHealth + ThisFrameShouldHeal,0.f,Character->Get_MaxHealth());

			Character->Set_CurrentHealth(CharacterCurrentHealth);
			Character->UpdateHealthHUD();

			if (CharacterCurrentHealth >= Character->Get_MaxHealth() || HealthAmount <= 0.f)
			{
				bIsHealthBuffActive = false;
			}
		}
	}
}

void UBuffComponent::InitDefaultSpeed(float WalkSpeed, float CrouchWalkSpeed)
{
	OriginWalkSpeed = WalkSpeed;
	OriginCrouchSpeed = CrouchWalkSpeed;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealBuffUpdate(DeltaTime);

}

