// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/HUD/BlasterHUD.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

UENUM()
enum EHUDType
{
	EHT_CarriedAmmo,
	EHT_CountDownTimer,
	EHT_WarmUpTimer,
	EHT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT()
struct FHUDData
{
	GENERATED_BODY()
	int32 CarriedAmmo;
	float CountDownTime;
};

USTRUCT()
struct FServerMatchState
{
	GENERATED_BODY()

	UPROPERTY()
	float MatchTime;
	UPROPERTY()
	float WarmUpTime;
	UPROPERTY()
	float LevelStartTime;
	UPROPERTY()
	FName MatchState;

	FServerMatchState(float MatchTime = 0.f, float WarmUpTime = 0.f, float LevelStartTime = 0.f,FName MatchState = MatchState::Aborted)
	{
		this->MatchTime = MatchTime;
		this->WarmUpTime = WarmUpTime;
		this->LevelStartTime = LevelStartTime;
		this->MatchState = MatchState;
	}
};

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	ABlasterHUD* BlasterHUD;

	/*
	 * 计时
	 */
	float MatchTime;
	float WarmUpTime;
	float LevelStartTime;
	uint32 CountDownSeconds;
	float SyncTimeFrequency = 5.f;
	float SyncTimeTimer;  //用于计时，到达frequency就进行一次同步
	double ServerClientDelta = 0.f;

	/*
	 * GameModeState
	 */
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void UpdateTimeHUD();
	void SyncServerTime(float DeltaSeconds);
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server,Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client,Reliable)
	void Client_ReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	UFUNCTION(Server,Reliable)
	void Server_RequestServerMatchState();

	UFUNCTION(Client,Reliable)
	void Client_ReportServerMatchState(const FServerMatchState& ServerMatchState);

	float GetServerTime();
public:
	void SetBlasterPlayerHealth(float Health,float MaxHealth);
	void SetBlasterPlayerScore(float Score);
	void SetBlasterPlayerDefeat(int32 Defeat);
	void SetBlasterPlayerAmmoAmount(int32 AmmoAmount);
	void SetBlasterPlayerHUDData(const EHUDType& HUDType,const FHUDData& Data);
	void SetAnnouncementHUDData(const EHUDType& HUDType,const FHUDData& Data);
	void OnMatchStateSet(FName State);
	void InitHUD();
};