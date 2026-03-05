// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/PlayerState/BlasterPlayerState.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

private:
	int TopScore;

public:
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeamPlayers;
	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeamPlayers;

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore;

	UPROPERTY(ReplicatedUsing=OnRep_BlueTeamScore)
	float BlueTeamScore;

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
	UFUNCTION()
	void OnRep_RedTeamScore();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateScore(ABlasterPlayerState* ScoringPlayer);
};
