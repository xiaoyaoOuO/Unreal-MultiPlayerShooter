// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Blast/PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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
	double TimeLeft = MatchTime - GetServerTime();
	uint32 MatchTimeLeft = FMath::CeilToInt(TimeLeft);
	if (MatchTimeLeft != CountDownSeconds)
	{
		CountDownSeconds = MatchTimeLeft;
		FHUDData HUDData;
		HUDData.CountDownTime = TimeLeft;
		SetBlasterPlayerHUDData(EHT_CountDownTimer,HUDData);
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

void ABlasterPlayerController::SetBlasterPlayerHUDData(const EHUDType& HUDType, const FHUDData& Data)
{
	BlasterHUD = BlasterHUD != nullptr ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD == nullptr) return;
	UCharacterOverlay* CharacterOverlay = BlasterHUD->GetCharacterOverlay();
	if (CharacterOverlay == nullptr) return;
	
	switch (HUDType)
	{
	case EHT_CarriedAmmo:
		if (CharacterOverlay->CarriedAmmoAmountText)
		{
			FString CarriedAmmoAmountString = FString::Printf(TEXT("%d"), Data.CarriedAmmo);
			CharacterOverlay->CarriedAmmoAmountText->SetText(FText::FromString(CarriedAmmoAmountString));
		}
		break;
	case EHT_CountDownTimer:
		if (CharacterOverlay->CountDownText)
		{
			uint32 Minute = FMath::FloorToInt(Data.CountDownTime / 60.f);
			uint32 Second = FMath::CeilToInt(Data.CountDownTime - Minute * 60);

			FString CountDownTimeString = FString::Printf(TEXT("%02d:%02d"), Minute, Second);
			CharacterOverlay->CountDownText->SetText(FText::FromString(CountDownTimeString));
		}
	default:
		break;
	}
}
