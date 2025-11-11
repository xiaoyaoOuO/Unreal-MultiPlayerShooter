// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiPlayerSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"


/**
 * 
 */
UCLASS()
class MULTIPLEPLAYERSPLUGINS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPlayers = 4,FString TypeOfMatch = "FreeForAll",FString LobbyPath = "/Game/ThirdPerson/Lobby");
	void MenuTeardown();
	virtual bool Initialize() override;
	
protected:
	virtual void NativeDestruct() override;
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult> & SearchResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton *JoinButton;

	UFUNCTION()
	void OnHostButtonClicked();
	UFUNCTION()
	void OnJoinButtonClicked();

	UMultiPlayerSessionSubsystem* MultiPlayerSessionSubsystem;

	int32 NumberOfPlayers;
	FString MatchType;
	FString PathToLobby;
};
