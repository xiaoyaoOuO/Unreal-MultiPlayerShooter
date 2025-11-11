// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerNums = GameState.Get()->PlayerArray.Num();
	if (PlayerNums >= 1)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FString Travel_Map = "/Game/Maps/BlasterMap";
			Travel_Map += "?listen";
			bUseSeamlessTravel = true;
			World->ServerTravel(Travel_Map);
		}
	}
}
