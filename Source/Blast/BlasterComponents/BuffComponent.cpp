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
	bIsHealthBuffActive = true;
	HealthRate = HealthAmount / BuffTime;
}

void UBuffComponent::SpeedBuff(float WalkSpeed, float CrouchWalkSpeed, float BuffTime)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->MaxWalkSpeed = WalkSpeed;
			CharacterMovementComponent->MaxWalkSpeedCrouched = CrouchWalkSpeed;
			MulticastSetSpeed(WalkSpeed, CrouchWalkSpeed);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SpeedBuffTimer,this,&UBuffComponent::ResetSpeed, BuffTime);
	}
}

void UBuffComponent::JumpBuff(float JumpZVelocity, float BuffTime)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->JumpZVelocity = JumpZVelocity;
			MulticastSetJump(JumpZVelocity);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(JumpBuffTimer,this,&UBuffComponent::ResetJump, BuffTime);
	}
}

void UBuffComponent::ShieldBuff(float Shield, float BuffTime)
{
	if (bIsShieldBuffActive) return;
	ShieldAmount = Shield;
	bIsShieldBuffActive = true;
	ShieldRate = ShieldAmount / BuffTime;
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

void UBuffComponent::ResetJump()
{
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->JumpZVelocity = OriginJumpZVelocity;
			MulticastSetJump(OriginJumpZVelocity);
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


void UBuffComponent::MulticastSetJump_Implementation(float JumpZVelocity)
{
	if (Character)
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			CharacterMovementComponent->JumpZVelocity = JumpZVelocity;
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
		Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
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

void UBuffComponent::ShieldBuffUpdate(float DeltaTime)
{
	if (bIsShieldBuffActive)
	{
		float ThisFrameShouldReplenish = ShieldRate * DeltaTime;
		Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
		if (Character)
		{
			ShieldAmount -= ThisFrameShouldReplenish;
			
			float CharacterCurrentShield = Character->Get_CurrentShield();
			CharacterCurrentShield = FMath::Clamp(CharacterCurrentShield + ThisFrameShouldReplenish,0.f,Character->Get_MaxShield());

			Character->Set_CurrentShield(CharacterCurrentShield);
			Character->UpdateShieldHUD();

			if (CharacterCurrentShield >= Character->Get_MaxShield() || ShieldAmount <= 0.f)
			{
				bIsShieldBuffActive = false;
			}
		}
	}
}

void UBuffComponent::InitDefaultSpeed(float WalkSpeed, float CrouchWalkSpeed)
{
	OriginWalkSpeed = WalkSpeed;
	OriginCrouchSpeed = CrouchWalkSpeed;
}

void UBuffComponent::InitDefaultJump(float JumpZVelocity)
{
	OriginJumpZVelocity = JumpZVelocity;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealBuffUpdate(DeltaTime);

	ShieldBuffUpdate(DeltaTime);

}

