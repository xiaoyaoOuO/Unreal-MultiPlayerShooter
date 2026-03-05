// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"

#include "Blast/GameState/BlasterGameState.h"

ATeamGameMode::ATeamGameMode()
{
	bTeamMatch = true;
}

void ATeamGameMode::AddPlayerToTeam(ABlasterGameState* BlasterGameState, ABlasterPlayerState* BlasterPlayerState)
{
	if (BlasterGameState->RedTeamPlayers.Num() <= BlasterGameState->BlueTeamPlayers.Num())
	{
		BlasterGameState->RedTeamPlayers.Add(BlasterPlayerState);
		BlasterPlayerState->SetTeam(ETeam::ET_Red);
	}
	else
	{
		BlasterGameState->BlueTeamPlayers.Add(BlasterPlayerState);
		BlasterPlayerState->SetTeam(ETeam::ET_Blue);
	}
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (NewPlayer == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	ABlasterPlayerState* BlasterPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
	if (BlasterGameState && BlasterPlayerState)
	{
		AddPlayerToTeam(BlasterGameState, BlasterPlayerState);
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	if (Exiting == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	ABlasterPlayerState* BlasterPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (BlasterGameState && BlasterPlayerState)
	{
		switch (ETeam PlayerTeam = BlasterPlayerState->GetTeam())
		{
		case ETeam::ET_Blue:
			if (BlasterGameState->BlueTeamPlayers.Contains(BlasterPlayerState))
			{
				BlasterGameState->BlueTeamPlayers.Remove(BlasterPlayerState);
			}
			break;
		case ETeam::ET_Red:
			if (BlasterGameState->RedTeamPlayers.Contains(BlasterPlayerState))
			{
				BlasterGameState->RedTeamPlayers.Remove(BlasterPlayerState);
			}
			break;
		default:
			break;
		}
	}
}

void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState == nullptr) return;

	for (auto Player : BlasterGameState->PlayerArray)
	{
		if (Player == nullptr) continue;
		if (ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(Player))
		{
			if (BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				AddPlayerToTeam(BlasterGameState, BlasterPlayerState);
			}
		}
	}
}

void ATeamGameMode::CharacterElim(class ABlasterCharacter* ElimmedCharacter,
	class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController)
{
	Super::CharacterElim(ElimmedCharacter, VictimController, AttackerController);

	if (AttackerController == nullptr || VictimController == nullptr) return;
	ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
	{
		BlasterGameState->AddTeamScore(VictimPlayerState, AttackerPlayerState);
	}
}
