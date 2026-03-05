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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	void AddPlayerScore(float AddScore);
	void AddPlayerDefeat(int32 AddDefeat);

	FORCEINLINE ETeam GetTeam() const {return Team;}
	void SetTeam(ETeam TeamToSet);

	UFUNCTION()
	virtual void OnRep_DefeatAmount();

	UFUNCTION()
	void OnRep_Team();
private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()
	ABlasterPlayerController* PlayerController;

	//State记录的玩家属性 
	UPROPERTY(ReplicatedUsing=OnRep_DefeatAmount)
	int32 DefeatAmount = 0;

	UPROPERTY(ReplicatedUsing=OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;
};
