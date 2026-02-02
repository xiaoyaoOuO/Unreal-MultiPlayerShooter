// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetBlasterPlayerHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->HealthBar && CharacterOverlay->HealthText)
		{
			float HealthPercent = Health / MaxHealth;
			CharacterOverlay->HealthBar->SetPercent(HealthPercent);
			FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
			CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
		}
	}
}
