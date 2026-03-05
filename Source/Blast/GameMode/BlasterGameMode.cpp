// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Blast/GameState/BlasterGameState.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Blast/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName CoolDown = FName(TEXT("CoolDown"));
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = MatchTime + WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}else if (MatchState == MatchState::CoolDown)
	{
		CountDownTime = CoolDownTime + MatchTime + WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayerController->OnMatchStateSet(MatchState, bTeamMatch);
		}
	}
}

//执行在服务端
void ABlasterGameMode::CharacterElim(ABlasterCharacter* ElimmedCharacter,
                                     ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	//角色死亡
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	if (VictimController == nullptr || AttackerController == nullptr) return;
	ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	
	if (VictimPlayerState && AttackerPlayerState && VictimPlayerState != AttackerPlayerState)
	{
		if (MatchState == MatchState::InProgress)
		{
			AttackerPlayerState->AddPlayerScore(ElimScore);
			if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
			{
				BlasterGameState->UpdateScore(AttackerPlayerState);
			}
		}
	}
	
	if (VictimPlayerState)
	{
		VictimPlayerState->AddPlayerDefeat(1);
	}
	
	//通知所有客户端角色被消灭了
	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayerController->Client_ElimAnnouncement(VictimPlayerState,AttackerPlayerState);
		}
	}
}

void ABlasterGameMode::RespawnCharacter(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter == nullptr || ElimmedController == nullptr) return;

	ElimmedCharacter->Reset();
	ElimmedCharacter->Destroy();

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),APlayerStart::StaticClass(),PlayerStarts);
	if (PlayerStarts.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0,PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[RandomIndex]);
	}
}

void ABlasterGameMode::PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	//先更新GameState中的TopScoringPlayers列表，移除离开的玩家
	if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
	{
		auto& TopScoringPlayers = BlasterGameState->TopScoringPlayers;
		if (TopScoringPlayers.Contains(PlayerLeaving))
		{
			TopScoringPlayers.Remove(PlayerLeaving);
		}
	}
	//再执行角色的消亡逻辑
	if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn()))
	{
		Multicast_EliminateCharacter(Character, true);
	}
}

void ABlasterGameMode::Multicast_EliminateCharacter_Implementation(ABlasterCharacter* ElimmedCharacter, bool bLeftGame)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(bLeftGame);
	}
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}
