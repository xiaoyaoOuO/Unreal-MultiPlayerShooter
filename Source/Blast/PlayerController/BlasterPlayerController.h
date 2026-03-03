// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/HUD/BlasterHUD.h"
#include "Blast/HUD/ReturnToMainMenu.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bIsHighPing);

UENUM()
enum EHUDType
{
	EHT_CarriedAmmo,   //玩家携带的弹药数量
	EHT_CountDownTimer, //比赛时的倒计时
	EHT_WarmUpTimer,	//比赛开始前的热身倒计时
	EHT_CoolDown,		//比赛结束后倒计时一段时间再重开比赛
	EHT_GrenadeAmount,  //玩家携带的手雷数量
	EHT_ShieldBar,      //玩家的护盾值
	EHT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT()
struct FHUDData
{
	GENERATED_BODY()
	int32 CarriedAmmo;
	int32 GrenadeAmount;
	int32 CurrentShield;
	int32 CurrentMaxShield;
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
	float CoolDownTime;
	
	UPROPERTY()
	FName MatchState;

	FServerMatchState() = default;

	FServerMatchState(float MatchTime, float WarmUpTime, float LevelStartTime,FName MatchState, float CoolDownTime)
	{
		this->MatchTime = MatchTime;
		this->WarmUpTime = WarmUpTime;
		this->LevelStartTime = LevelStartTime;
		this->MatchState = MatchState;
		this->CoolDownTime = CoolDownTime;
	}
};

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	float SoloTripTime; //一次PRC发送的时间，武器开火调用RPC需要在服务端做开火校验，但是传入的HitTime是客户端的时间，所以在服务器校验时需要将客户端的HitTime转换为服务器时间，SoloTripTime就是这个转换的时间

	//高Ping的委托
	FHighPingDelegate HighPingDelegate;
	
private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;
	
	/*
	 * 计时
	 */
	float MatchTime;
	float WarmUpTime;
	float LevelStartTime;
	float CoolDownTime;
	uint32 CountDownSeconds;
	float SyncTimeFrequency = 5.f;
	float SyncTimeTimer;  //用于计时，到达frequency就进行一次同步
	double ServerClientDelta = 0.f;


	//初始化HUD
	bool bHasInitGrenade;
	bool bHasInitShield;
	bool bHasInitAmmo;

	/*
	 * GameModeState
	 */
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	/*
	 * 高Ping的警告
	 */
	float CurrentPing;
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 1.f;
	UPROPERTY(EditAnywhere)
	float UpdatePingFrequency = 2.f;
	float UpdatePingTimer = 0.f;
	//隔一段时间检查一次，如果ping过高就显示警告
	UPROPERTY(EditAnywhere)
	float PingWarningFrequency = 20.f;
	UPROPERTY(EditAnywhere)
	float PingAnimationDuration = 1.f;
	float PingWarningTimer = 0.f;
	float PingWarningAnimationTimer = 0.f;

	//返回主菜单的界面
	UPROPERTY(EditAnywhere,Category="Widget")
	TSubclassOf<UUserWidget> ReturnToMainMenuWidgetClass;
	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenuWidget;
	bool bReturnMenuOpen = false;
	
	void HandleMatchStarted();
	void HandleCoolDown();
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
	virtual void SetupInputComponent() override;

	UFUNCTION(Server,Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client,Reliable)
	void Client_ReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	UFUNCTION(Server,Reliable)
	void Server_RequestServerMatchState();

	UFUNCTION(Client,Reliable)
	void Client_ReportServerMatchState(const FServerMatchState& ServerMatchState);

	UFUNCTION(Server,Reliable)
	void Server_ReportPingState(bool bIsHighPing);

public:
	float GetServerTime();
	void SetBlasterPlayerHealth(float Health,float MaxHealth);
	void SetBlasterPlayerScore(float Score);
	void SetBlasterPlayerDefeat(int32 Defeat);
	void SetBlasterPlayerAmmoAmount(int32 AmmoAmount);
	void UpdateCarriedAmmo(const FHUDData& Data, UCharacterOverlay* CharacterOverlay);
	void UpdateCountDown(const FHUDData& Data, UCharacterOverlay* CharacterOverlay);
	void UpdateGrenadeAmount(const FHUDData& Data, UCharacterOverlay* CharacterOverlay);
	void UpdateShield(const FHUDData& Data, UCharacterOverlay* CharacterOverlay);
	void SetBlasterPlayerHUDData(const EHUDType& HUDType,const FHUDData& Data);
	void SetAnnouncementHUDData(const EHUDType& HUDType,const FHUDData& Data);
	void OnMatchStateSet(FName State);
	void InitHUD();
	void DrawCoolDownHUD(const UAnnouncement* Announcement,const FHUDData& Data);
	void UpdateCharacterShield(float CurrentShield, float MaxShield);
	void PollInit();
	void UpdatePingHUD();
	void UpdatePingWarning(float DeltaSeconds);
	void PollForPing(float DeltaSeconds);
	void OnQuitButtonPressed();
};