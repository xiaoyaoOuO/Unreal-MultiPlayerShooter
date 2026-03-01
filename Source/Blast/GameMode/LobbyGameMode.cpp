// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (bTravelInProgress || !GameState)
	{
		return;
	}

	const int32 PlayerNums = GameState->PlayerArray.Num();
	if (PlayerNums >= 2)
	{
		bTravelInProgress = true;
		HandleLobbyTravel();
	}
}

void ALobbyGameMode::HandleLobbyTravel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		bTravelInProgress = false;
		return;
	}

	GetWorldTimerManager().ClearTimer(TravelTimerHandle);

	const FString TravelMap = TEXT("/Game/Maps/BlasterMap?listen");
	bUseSeamlessTravel = true;
	World->ServerTravel(TravelMap);
}
