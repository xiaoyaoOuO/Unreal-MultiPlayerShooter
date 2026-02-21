// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"

#include "Blast/BlasterComponents/CombatComponent.h"
#include "Blast/Character/BlasterCharacter.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UCombatComponent* CombatComponent = BlasterCharacter->GetCombatComponent())
		{
			CombatComponent->PickUpAmmo(AmmoType,AmmoAmount);
			Destroy();
		}
	}
}

void AAmmoPickUp::Destroyed()
{
	Super::Destroyed();
}
