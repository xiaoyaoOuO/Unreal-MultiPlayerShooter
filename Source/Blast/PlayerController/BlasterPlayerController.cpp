// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetBlasterPlayerHealth(BlasterCharacter->Get_CurrentHealth(),BlasterCharacter->Get_MaxHealth());
	}
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

void ABlasterPlayerController::SetBlasterPlayerScore(float Score)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->ScoreText)
		{
			FString ScoreString = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
			CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreString));
		}
	}
}

void ABlasterPlayerController::SetBlasterPlayerDefeat(int32 Defeat)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->DefeatText)
		{
			FString DefeatString = FString::Printf(TEXT("%d"), Defeat);
			CharacterOverlay->DefeatText->SetText(FText::FromString(DefeatString));
		}
	}
}

void ABlasterPlayerController::SetBlasterPlayerAmmoAmount(int32 AmmoAmount)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->AmmoAmountText)
		{
			FString AmmoAmountString = FString::Printf(TEXT("%d"), AmmoAmount);
			CharacterOverlay->AmmoAmountText->SetText(FText::FromString(AmmoAmountString));
		}
	}
}
