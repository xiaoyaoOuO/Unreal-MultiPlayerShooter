// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void CharacterElim(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);
	virtual void RespawnCharacter(ACharacter* ElimmedCharacter, AController* VictimController);
private:
	float ElimScore = 1.f;
};
