// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLAST_API const FName CoolDown;
}
/**
 * 
 */
UCLASS()
class BLAST_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
	virtual void CharacterElim(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);
	virtual void RespawnCharacter(ACharacter* ElimmedCharacter, AController* VictimController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EliminateCharacter(ABlasterCharacter* ElimmedCharacter, bool bLeftGame);

	ABlasterGameMode();

protected:
	bool bTeamMatch = false;
	
private:
	float ElimScore = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f;
	
	float CountDownTime = 0.f;
	float LevelStartTime = 0.f;

public:
	FORCEINLINE float GetElimScore() const { return ElimScore; }
	FORCEINLINE float GetWarmUpTime() const { return WarmUpTime; }
	FORCEINLINE float GetMatchTime() const { return MatchTime; }
	FORCEINLINE float GetLevelStartTime() const { return LevelStartTime; }
	FORCEINLINE float GetCoolDownTime() const { return CoolDownTime; }
};
