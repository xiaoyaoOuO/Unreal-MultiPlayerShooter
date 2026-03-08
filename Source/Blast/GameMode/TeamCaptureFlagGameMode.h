// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "TeamCaptureFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ATeamCaptureFlagGameMode : public ATeamGameMode
{
	GENERATED_BODY()

public:
	void FlagCaptured(ABlasterCharacter* CapturingCharacter);
	
};
