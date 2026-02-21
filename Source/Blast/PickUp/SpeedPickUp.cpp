// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickUp.h"

#include "Blast/BlasterComponents/BuffComponent.h"
#include "Blast/Character/BlasterCharacter.h"

void ASpeedPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->SpeedBuff(BuffWalkSpeed, BuffCrouchWalkSpeed, BuffTime);
			Destroy();
		}
	}
}
