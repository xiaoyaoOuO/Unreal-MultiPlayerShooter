// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

ULagCompensationComponent::ULagCompensationComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}



void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);
	ShowFramePackage(FramePackage);
}



void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		for (const auto& HitBox : Character->HitBoxComponentMap)
		{
			FBoxInformation BoxInformation;
			Package.Time = GetWorld()->GetTimeSeconds();
			if (const UBoxComponent* Box = HitBox.Value)
			{
				BoxInformation.BoxLocation = Box->GetComponentLocation();
				BoxInformation.BoxRotation = Box->GetComponentRotation();
				BoxInformation.BoxExtent = Box->GetScaledBoxExtent();
			}
			Package.HitBoxInfo.Add(HitBox.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package)
{
	for (const auto& HitBox : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),
			HitBox.Value.BoxLocation,
			HitBox.Value.BoxExtent,
			FQuat(HitBox.Value.BoxRotation),
			FColor::Red,
			true
		);
	}
}

