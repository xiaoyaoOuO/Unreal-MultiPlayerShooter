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

void ABlasterPlayerState::UpdatePlayerScore(float NewScore)
{
	SetScore(NewScore);
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
