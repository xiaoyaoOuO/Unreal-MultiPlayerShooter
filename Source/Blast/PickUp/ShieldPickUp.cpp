// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickUp.h"

void AShieldPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->ShieldBuff(ReplenishShield, ReplenishTime);
			Destroy();
		}
	}
}
