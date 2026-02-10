// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Blast/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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
			BlasterPlayerController->OnMatchStateSet(MatchState);
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
		ElimmedCharacter->Elim();
	}
	if (AttackerController)
	{
		ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>();
		ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();
		if (VictimPlayerState && AttackerPlayerState && VictimPlayerState != AttackerPlayerState)
		{
			AttackerPlayerState->AddPlayerScore(ElimScore);
		}
	}
	if (VictimController)
	{
		if (ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>())
		{
			VictimPlayerState->AddPlayerDefeat(1);
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

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}
