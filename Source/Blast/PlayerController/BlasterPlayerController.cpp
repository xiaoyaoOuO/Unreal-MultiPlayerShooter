// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blast/BlasterComponents/CombatComponent.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Blast/GameMode/BlasterGameMode.h"
#include "Blast/GameState/BlasterGameState.h"
#include "Blast/PlayerState/BlasterPlayerState.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::HandleMatchStarted()
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		BlasterHUD->CloseAnnouncement();
		BlasterHUD->AddCharacterOverlay();
		InitHUD();
	}
}

void ABlasterPlayerController::HandleCoolDown()
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		if (UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay())
		{
			CharacterOverlay->RemoveFromParent();
		}
		BlasterHUD->OpenAnnouncement();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
		if (BlasterHUD)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchStarted();
	}else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	Server_RequestServerMatchState();
	Server_RequestServerTime(GetWorld()->GetTimeSeconds());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetBlasterPlayerHealth(BlasterCharacter->Get_CurrentHealth(),BlasterCharacter->Get_MaxHealth());
	}
}

void ABlasterPlayerController::UpdateTimeHUD()
{
	double TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = MatchTime + WarmUpTime - GetServerTime() + LevelStartTime;
	else if (MatchState == MatchState::CoolDown) TimeLeft = CoolDownTime + MatchTime + WarmUpTime - GetServerTime() + LevelStartTime;

	uint32 MatchTimeLeft = FMath::CeilToInt(TimeLeft);

	if (MatchTimeLeft != CountDownSeconds)
	{
		FHUDData HUDData;
		HUDData.CountDownTime = TimeLeft;
		if (MatchState == MatchState::WaitingToStart)
		{
			SetAnnouncementHUDData(EHT_WarmUpTimer,HUDData);
		}else if (MatchState == MatchState::InProgress)
		{
			SetBlasterPlayerHUDData(EHT_CountDownTimer,HUDData);
		}else if (MatchState == MatchState::CoolDown)
		{
			SetAnnouncementHUDData(EHT_CoolDown,HUDData);
		}
	}
}

void ABlasterPlayerController::SyncServerTime(float DeltaSeconds)
{
	if (IsLocalController() && !HasAuthority())
	{
		SyncTimeTimer += DeltaSeconds;
		if (SyncTimeTimer >= SyncTimeFrequency)
		{
			Server_RequestServerTime(GetWorld()->GetTimeSeconds());
			SyncTimeTimer = 0.f;
		}
	}
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SyncServerTime(DeltaSeconds);
	
	UpdateTimeHUD();

	PollInit();

	PollForPing(DeltaSeconds);	
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController() && !HasAuthority())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTime = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeOfClientRequest,ServerTime);
}

void ABlasterPlayerController::Client_ReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	double CurrentTime = GetWorld()->GetTimeSeconds();
	double RPCDelay = CurrentTime - TimeOfClientRequest;
	double CurrentServerTime = TimeServerReceivedClientRequest + RPCDelay * 0.5f;
	ServerClientDelta = CurrentServerTime - CurrentTime;
}

void ABlasterPlayerController::Server_RequestServerMatchState_Implementation()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(BlasterGameMode))
	{
		MatchTime = BlasterGameMode->GetMatchTime();
		WarmUpTime = BlasterGameMode->GetWarmUpTime();
		LevelStartTime = BlasterGameMode->GetLevelStartTime();
		MatchState = BlasterGameMode->GetMatchState();
		CoolDownTime = BlasterGameMode->GetCoolDownTime();

		Client_ReportServerMatchState(FServerMatchState(MatchTime,WarmUpTime,LevelStartTime,MatchState,CoolDownTime));
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("GameMode is not valid"));
	}
}

void ABlasterPlayerController::Client_ReportServerMatchState_Implementation(const FServerMatchState& ServerMatchState)
{
	MatchTime = ServerMatchState.MatchTime;
	WarmUpTime = ServerMatchState.WarmUpTime;
	LevelStartTime = ServerMatchState.LevelStartTime;
	CoolDownTime = ServerMatchState.CoolDownTime;
	OnMatchStateSet(ServerMatchState.MatchState);
	UE_LOG(LogTemp,Warning,TEXT("Client : MatchTime: %f, WarmUpTime: %f, LevelStartTime: %f"), MatchTime, WarmUpTime, LevelStartTime);
}

float ABlasterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ServerClientDelta;
}

void ABlasterPlayerController::SetBlasterPlayerHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->HealthBar && CharacterOverlay->HealthText)
		{
			if (MaxHealth <= 0)
			{
				CharacterOverlay->HealthBar->SetPercent(0.f);
				CharacterOverlay->HealthText->SetText(FText::FromString(TEXT("0/0")));
				return;
			}
			float HealthPercent = Health / MaxHealth;
			CharacterOverlay->HealthBar->SetPercent(HealthPercent);
			FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
			CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
		}
	}
}

void ABlasterPlayerController::SetBlasterPlayerScore(float Score)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->ScoreText)
		{
			FString ScoreString = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
			CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreString));
		}
	}
}

void ABlasterPlayerController::SetBlasterPlayerDefeat(int32 Defeat)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->DefeatText)
		{
			FString DefeatString = FString::Printf(TEXT("%d"), Defeat);
			CharacterOverlay->DefeatText->SetText(FText::FromString(DefeatString));
		}
	}
}

void ABlasterPlayerController::SetBlasterPlayerAmmoAmount(int32 AmmoAmount)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
		if (CharacterOverlay && CharacterOverlay->AmmoAmountText)
		{
			FString AmmoAmountString = FString::Printf(TEXT("%d"), AmmoAmount);
			CharacterOverlay->AmmoAmountText->SetText(FText::FromString(AmmoAmountString));
		}
	}
}

void ABlasterPlayerController::UpdateCarriedAmmo(const FHUDData& Data, UCharacterOverlay* CharacterOverlay)
{
	if (CharacterOverlay->CarriedAmmoAmountText)
	{
		FString CarriedAmmoAmountString = FString::Printf(TEXT("%d"), Data.CarriedAmmo);
		CharacterOverlay->CarriedAmmoAmountText->SetText(FText::FromString(CarriedAmmoAmountString));
		bHasInitAmmo = true;
	}
}

void ABlasterPlayerController::UpdateCountDown(const FHUDData& Data, UCharacterOverlay* CharacterOverlay)
{
	if (CharacterOverlay->CountDownText)
	{
		uint32 Minute = FMath::FloorToInt(Data.CountDownTime / 60.f);
		uint32 Second = FMath::CeilToInt(Data.CountDownTime - Minute * 60);

		FString CountDownTimeString = FString::Printf(TEXT("%02d:%02d"), Minute, Second);
		if (Data.CountDownTime <= 0.f)
		{
			CountDownTimeString = FString("");
		}
		CharacterOverlay->CountDownText->SetText(FText::FromString(CountDownTimeString));
	}
}

void ABlasterPlayerController::UpdateGrenadeAmount(const FHUDData& Data, UCharacterOverlay* CharacterOverlay)
{
	if (CharacterOverlay->GrenadeAmountText)
	{
		FString CarriedGrenadeAmountString = FString::Printf(TEXT("%d"), Data.GrenadeAmount);
		CharacterOverlay->GrenadeAmountText->SetText(FText::FromString(CarriedGrenadeAmountString));
		bHasInitGrenade = true;
	}
}

void ABlasterPlayerController::UpdateShield(const FHUDData& Data, UCharacterOverlay* CharacterOverlay)
{
	if (CharacterOverlay->ShieldBar && CharacterOverlay->ShieldText)
	{
		bHasInitShield = true;
		if (Data.CurrentMaxShield <= 0)
		{
			CharacterOverlay->ShieldBar->SetPercent(0.f);
			CharacterOverlay->ShieldText->SetText(FText::FromString(TEXT("0/0")));
			return;
		}
		float ShieldPercent = static_cast<float>(Data.CurrentShield) / static_cast<float>(Data.CurrentMaxShield);
		UE_LOG(LogTemp,Warning,TEXT("Shield Percent = %f"), ShieldPercent);
		CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), Data.CurrentShield, Data.CurrentMaxShield);
		CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
}

void ABlasterPlayerController::UpdatePingHUD()
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD == nullptr) return;
	UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
	if (CharacterOverlay == nullptr) return;
	
	if (CharacterOverlay->PingText)
	{
		if (APlayerState* BlasterPlayerState = GetPlayerState<APlayerState>())
		{
			CurrentPing = BlasterPlayerState->GetCompressedPing() * 4.f;
			FString PingString = FString::Printf(TEXT("%d ms"), FMath::CeilToInt(CurrentPing));
			CharacterOverlay->PingText->SetText(FText::FromString(PingString));
		}
	}
}

void ABlasterPlayerController::UpdatePingWarning(float DeltaSeconds)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD == nullptr) return;
	UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
	if (CharacterOverlay == nullptr) return;
	
	if (CharacterOverlay->IsAnimationPlaying(CharacterOverlay->HighPingWarningAnimation))
	{
		PingWarningAnimationTimer += DeltaSeconds;
		if (PingWarningAnimationTimer >= PingAnimationDuration)
		{
			if (CharacterOverlay->HighPingImage)
			{
				CharacterOverlay->HighPingImage->SetOpacity(0.f);
			}
			CharacterOverlay->StopAnimation(CharacterOverlay->HighPingWarningAnimation);
			PingWarningAnimationTimer = 0.f;
		}
	}else
	{
		PingWarningTimer += DeltaSeconds;
		if (PingWarningTimer >= PingWarningFrequency)
		{
			if (CurrentPing > HighPingThreshold)
			{
				if (CharacterOverlay->HighPingImage)
				{
					CharacterOverlay->HighPingImage->SetOpacity(1.f);
				}
				CharacterOverlay->PlayAnimation(CharacterOverlay->HighPingWarningAnimation, 0.f, 0);
				PingWarningTimer = 0.f;
			}
		}
	}
}

void ABlasterPlayerController::PollForPing(float DeltaSeconds)
{
	if (UpdatePingTimer >= UpdatePingFrequency)
	{
		UpdatePingHUD();
		UpdatePingTimer = 0.f;
	}
	else
	{
		UpdatePingTimer += DeltaSeconds;
	}

	UpdatePingWarning(DeltaSeconds);
}

void ABlasterPlayerController::SetBlasterPlayerHUDData(const EHUDType& HUDType, const FHUDData& Data)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD == nullptr) return;
	UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
	if (CharacterOverlay == nullptr) return;
	
	switch (HUDType)
	{
	case EHT_CarriedAmmo:
		UpdateCarriedAmmo(Data, CharacterOverlay);
		break;
	case EHT_CountDownTimer:
		UpdateCountDown(Data, CharacterOverlay);
		break;
	case EHT_GrenadeAmount:
		UpdateGrenadeAmount(Data, CharacterOverlay);
		break;
	case EHT_ShieldBar:
		UpdateShield(Data, CharacterOverlay);
		break;
	default:
		break;
	}
}

void ABlasterPlayerController::SetAnnouncementHUDData(const EHUDType& HUDType, const FHUDData& Data)
{
	/*先检查BlasterHUD和Announcement是否有效*/
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD == nullptr) return;
	UAnnouncement* Announcement = BlasterHUD->GetAnnouncement();
	if (Announcement == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("Announcement is nullptr"));
		return;
	}

	/*根据HUDType设置不同的文本*/
	switch (HUDType)
	{
	case EHT_WarmUpTimer:
		if (Announcement->WarmUpTimerText)
		{
			uint32 Minute = FMath::FloorToInt(Data.CountDownTime / 60.f);
			uint32 Second = FMath::CeilToInt(Data.CountDownTime - Minute * 60);

			FString CountDownTimeString = FString::Printf(TEXT("%02d:%02d"), Minute, Second);
			if (Data.CountDownTime <= 0.f)
			{
				CountDownTimeString = FString("");
			}
			Announcement->WarmUpTimerText->SetText(FText::FromString(CountDownTimeString));
		}
		if (Announcement->TitleText)
		{
			Announcement->TitleText->SetText(FText::FromString(TEXT("热身阶段")));
		}
		if (Announcement->InfoText)
		{
			Announcement->InfoText->SetText(FText::FromString(TEXT("准备好迎接战斗了么?")));
		}
		break;
	case EHT_CoolDown:
		DrawCoolDownHUD(Announcement,Data);
		break;
	default:
		break;
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
		if (BlasterHUD)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchStarted();
	}else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::InitHUD()
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter());
	if (BlasterCharacter)
	{
		BlasterCharacter->UpdateHealthHUD();
		BlasterCharacter->UpdateShieldHUD();
		BlasterCharacter->PollInit();
	}
}

void ABlasterPlayerController::DrawCoolDownHUD(const UAnnouncement* Announcement,const FHUDData& Data)
{
	if (Announcement->WarmUpTimerText)
	{
		uint32 Minute = FMath::FloorToInt(Data.CountDownTime / 60.f);
		uint32 Second = FMath::CeilToInt(Data.CountDownTime - Minute * 60);

		FString CountDownTimeString = FString::Printf(TEXT("%02d:%02d"), Minute, Second);
		if (Data.CountDownTime <= 0.f)
		{
			CountDownTimeString = FString("");
		}
		Announcement->WarmUpTimerText->SetText(FText::FromString(CountDownTimeString));
	}
	if (Announcement->TitleText)
	{
		FText TitleString = FText::FromString(TEXT("比赛结束,倒计时结束重新开始"));
		Announcement->TitleText->SetText(TitleString);
	}
	if (Announcement->InfoText)
	{
		if (ABlasterGameState* GameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
		{
			UE_LOG(LogTemp,Warning,TEXT("UpdateScore With GameState"));
			TArray<ABlasterPlayerState*> ScorePlayers = GameState->TopScoringPlayers;
			FText InfoText;
			if (ScorePlayers.Num() == 0)
			{
				InfoText = FText::FromString(TEXT("没有玩家得分"));
			}else if (ScorePlayers.Num() == 1)
			{
				FString WinnerName = ScorePlayers[0]->GetPlayerName();
				InfoText = FText::FromString(FString::Printf(TEXT("胜利者: %s"), *WinnerName));
			}else
			{
				FString WinnerNames;
				for (const auto BlasterPlayerState : ScorePlayers)
				{
					WinnerNames += BlasterPlayerState->GetPlayerName() + TEXT("\n");
				}
				InfoText = FText::FromString(FString::Printf(TEXT("胜利者:\n %s"), *WinnerNames));
			}
			Announcement->InfoText->SetText(InfoText);
		}
	}
}

void ABlasterPlayerController::UpdateCharacterShield(float CurrentShield, float MaxShield)
{
	FHUDData HUDData;
	HUDData.CurrentShield = CurrentShield;
	HUDData.CurrentMaxShield = MaxShield;
	SetBlasterPlayerHUDData(EHT_ShieldBar,HUDData);
}

void ABlasterPlayerController::PollInit()
{
	if (!bHasInitAmmo)
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter()))
		{
			if (UCombatComponent* CombatComponent = BlasterCharacter->GetCombatComponent())
			{
				CombatComponent->UpdateCarriedAmmoHUD();
			}
		}
	}
	if (!bHasInitGrenade)
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter()))
		{
			if (UCombatComponent* CombatComponent = BlasterCharacter->GetCombatComponent())
			{
				CombatComponent->UpdateGrenadeHUD();
			}
		}
	}
	if (!bHasInitShield)
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter()))
		{
			BlasterCharacter->UpdateShieldHUD();
		}
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}
