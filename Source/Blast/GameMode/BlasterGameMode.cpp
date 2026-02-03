// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blast/Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::CharacterElim(ABlasterCharacter* ElimmedCharacter,
                                     ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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
