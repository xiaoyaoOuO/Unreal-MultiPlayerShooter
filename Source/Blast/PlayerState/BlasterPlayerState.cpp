// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		PlayerController = PlayerController != nullptr ? PlayerController : Cast<ABlasterPlayerController>(Character->Controller);
		if (PlayerController)
		{
			PlayerController->SetBlasterPlayerScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddPlayerScore(float AddScore)
{
	SetScore(GetScore() + AddScore);
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		PlayerController = PlayerController != nullptr ? PlayerController : Cast<ABlasterPlayerController>(Character->Controller);
		if (PlayerController)
		{
			PlayerController->SetBlasterPlayerScore(GetScore());
		}
	}
}
