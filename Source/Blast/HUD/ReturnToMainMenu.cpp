// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "Blast/PlayerController/BlasterPlayerController.h"

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize()) return false;
	return true;
}

void UReturnToMainMenu::MenuStart()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (UWorld* World = GetWorld())
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(World->GetFirstPlayerController()))
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			BlasterPlayerController->SetInputMode(InputMode);
			BlasterPlayerController->SetShowMouseCursor(true);
		}
	}
	
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::OnReturnButtonClicked);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this,&UReturnToMainMenu::OnDestroySessionComplete);
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	if (UWorld* World = GetWorld())
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(World->GetFirstPlayerController()))
		{
			FInputModeGameOnly InputMode;
			BlasterPlayerController->SetInputMode(InputMode);
			BlasterPlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::OnReturnButtonClicked);
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UReturnToMainMenu::OnReturnButtonClicked()
{
	if (ReturnButton)
	{
		ReturnButton->SetIsEnabled(false);
	}
	if (UWorld* World = GetWorld())
	{
		if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(World->GetFirstPlayerController()))
		{
			if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(PlayerController->GetCharacter()))
			{
				Character->ServerLeaveGame();
				Character->OnLeaveGame.AddDynamic(this,&UReturnToMainMenu::OnPlayerLeftGame);
			}else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UReturnToMainMenu::OnDestroySessionComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		if (ReturnButton)
		{
			ReturnButton->SetIsEnabled(true);
		}
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->ReturnToMainMenuHost();
		}else
		{
			if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(World->GetFirstPlayerController()))
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

