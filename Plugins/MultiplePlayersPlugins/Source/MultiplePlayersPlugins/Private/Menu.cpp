// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "SkeletalMeshAttributes.h"
#include "Components/Button.h"

void UMenu::MenuSetup(int32 NumbersOfPublicConnection,FString TypeOfMatch,FString LobbyPath)
{
	this->PathToLobby = LobbyPath;
	this->NumberOfPlayers = NumbersOfPublicConnection;
	this->MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);   //直接访问bIsFocusable已被弃用

	UWorld *World = GetWorld();
	if (World)
	{
		APlayerController *PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputModeData.SetWidgetToFocus(TakeWidget());//获取底层的 Slate 小部件，如果不存在则构造它。
			//如果你想替换所构造的 Slate 小部件，可以查找 RebuildWidget。
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	UGameInstance *GameInstance = GetGameInstance();
	if (GameInstance)
	{
		this->MultiPlayerSessionSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
	}

	if (this->MultiPlayerSessionSubsystem)
	{
		//加入委托，创建会话后广播调用
		MultiPlayerSessionSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this,&ThisClass::OnCreateSession);
		MultiPlayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this,&ThisClass::OnDestroySession);
		MultiPlayerSessionSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this,&ThisClass::OnStartSession);
		MultiPlayerSessionSubsystem->MultiPlayerOnFindSessionCompleteDelegate.AddUObject(this,&ThisClass::OnFindSession);
		MultiPlayerSessionSubsystem->MultiPlayerOnJoinSessionCompleteDelegate.AddUObject(this,&ThisClass::OnJoinSession);
	}
}

void UMenu::MenuTeardown()
{
	RemoveFromParent();

	UWorld *World = GetWorld();
	if (World)
	{
		APlayerController *PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;	
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::OnHostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenu::OnJoinButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTeardown();
	Super::NativeDestruct();
}

//多播委托FMultiPlayerOnCreateSessionComplete调用的函数
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	//创建会话成功
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				10.f,
				FColor::Green,
				FString::Printf(TEXT("create session success!"))
			);
		}
		if (UWorld *World = GetWorld())
		{
			FString LobbyPath = FString::Printf(TEXT("%s?listen"),*PathToLobby);
			World->ServerTravel(LobbyPath);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				10.f,
				FColor::Blue,
				FString::Printf(TEXT("create session fail"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (!MultiPlayerSessionSubsystem)
	{
		return;
	}
	for (auto & Result : SearchResults)
	{
		FString SettingName;
		Result.Session.SessionSettings.Get(FName("MatchType"),SettingName);
		if (MatchType.Equals(SettingName))
		{
			MultiPlayerSessionSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SearchResults.Num() <= 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		FString Address;
		SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);

		APlayerController *PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			10.f,
			FColor::Blue,
			FString::Printf(TEXT("create button is clicked"))
		);
	}
	if (MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->CreateSession(this->NumberOfPlayers,this->MatchType);
	}
	
}

void UMenu::OnJoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->FindSession(10000);
	}
}
