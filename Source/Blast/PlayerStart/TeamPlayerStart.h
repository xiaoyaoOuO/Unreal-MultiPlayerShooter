// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/BlasterType/TeamType.h"
#include "GameFramework/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	ETeam Team = ETeam::ET_NoTeam;

	FORCEINLINE ETeam GetTeam() const {return Team;}
};
