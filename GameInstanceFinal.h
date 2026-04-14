// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "GameInstanceFinal.generated.h"

/**
 * 
 */
UCLASS()
class DC_MULTIJUGADOR_API UGameInstanceFinal : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void CreateAdvancedSessionCPlusPlus(int32 PublicConnections = 4, int32 PrivateConnections = 0);


private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	IOnlineSessionPtr SessionInterface;
	FDelegateHandle CreateSessionDelegateHandle;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
};
