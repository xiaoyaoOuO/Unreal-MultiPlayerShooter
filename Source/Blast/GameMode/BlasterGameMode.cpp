// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blast/Character/BlasterCharacter.h"

void ABlasterGameMode::CharacterElim(ABlasterCharacter* ElimmedCharacter,
                                     ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
