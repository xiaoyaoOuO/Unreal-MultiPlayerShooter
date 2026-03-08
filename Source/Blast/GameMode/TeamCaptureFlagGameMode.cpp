// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamCaptureFlagGameMode.h"

void ATeamCaptureFlagGameMode::FlagCaptured(ABlasterCharacter* CapturingCharacter)
{
	if (CapturingCharacter && CapturingCharacter->IsHoldingFlag())
	{
		if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
		{
			ABlasterPlayerState* CapturingPlayerState = CapturingCharacter->GetPlayerState<ABlasterPlayerState>();
			BlasterGameState->AddTeamScore(CapturingPlayerState,5);
			CapturingCharacter->DropFlag(true);
		}
	}
}
