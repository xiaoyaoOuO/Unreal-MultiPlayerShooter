// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickUp.h"

#include "NiagaraComponent.h"
#include "Blast/BlasterComponents/BuffComponent.h"
#include "Blast/BlasterComponents/CombatComponent.h"
#include "Kismet/GameplayStatics.h"


void AHealthPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->HealthBuff(HealAmount, HealTime);
			Destroy();
		}
	}
}

void AHealthPickUp::Destroyed()
{
	Super::Destroyed();
}
