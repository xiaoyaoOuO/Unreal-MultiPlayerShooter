// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "Blast/GameState/BlasterGameState.h"
#include "TeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ATeamGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	void AddPlayerToTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState);
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleMatchHasStarted() override;
};
