// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void OnRep_Score() override;
	void UpdatePlayerScore(float NewScore);
private:
	ABlasterCharacter* Character;
	ABlasterPlayerController* PlayerController;
};
