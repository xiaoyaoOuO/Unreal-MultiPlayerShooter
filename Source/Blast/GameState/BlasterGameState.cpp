// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Net/UnrealNetwork.h"

void ABlasterGameState::OnRep_BlueTeamScore()
{
}

void ABlasterGameState::OnRep_RedTeamScore()
{
}

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateScore(ABlasterPlayerState* ScoringPlayer)
{
	bool bHaveNewTopScorer = false;
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
		bHaveNewTopScorer = true;
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		//如果当前玩家的分数超过了之前的最高分，那么之前的最高分玩家就失去领先地位，当前玩家成为新的最高分玩家
		for (const auto TopScoringPlayer : TopScoringPlayers)
		{
			if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(TopScoringPlayer->GetPawn()))
			{
				BlasterCharacter->Multicast_CharacterLostLead();
			}
		}
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		bHaveNewTopScorer = true;
	}
	if (bHaveNewTopScorer) //只添加新玩家的领先特效
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ScoringPlayer->GetPawn()))
		{
			BlasterCharacter->Multicast_CharacterGainedLead();
		}
	}
}
