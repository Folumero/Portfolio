// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstanceFinal.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"


void UGameInstanceFinal::CreateAdvancedSessionCPlusPlus(int32 PublicConnections, int32 PrivateConnections)
{

	    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (!SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("No se pudo obtener el SessionInterface"));
            return;
        }

        // 2. Si ya existe una sesión con ese nombre, destruirla
        if (SessionInterface->GetNamedSession(NAME_GameSession))
        {
            SessionInterface->DestroySession(NAME_GameSession);
        }

        // 3. Configurar las opciones (lo que marcan los checkboxes del nodo)
        SessionSettings = MakeShared<FOnlineSessionSettings>();
        SessionSettings->NumPublicConnections = PublicConnections;
        SessionSettings->NumPrivateConnections = PrivateConnections;

        SessionSettings->bIsLANMatch                      = false; 
        SessionSettings->bAllowInvites                    = true;  
        SessionSettings->bIsDedicated                     = false; 
        SessionSettings->bUseLobbiesIfAvailable           = true;  
        SessionSettings->bAllowJoinViaPresence            = true;  
        SessionSettings->bAllowJoinViaPresenceFriendsOnly = false; 
        SessionSettings->bAntiCheatProtected              = false; 
        SessionSettings->bUsesStats                       = false; 
        SessionSettings->bShouldAdvertise                 = true;  
        SessionSettings->bUseLobbiesVoiceChatIfAvailable  = true;  
        SessionSettings->bAllowJoinInProgress             = true;
        SessionSettings->bUsesPresence                    = true;  

        // Puedes añadir "Extra Settings" (clave/valor)
        SessionSettings->Set(FName("SERVER_NAME_KEY"), FString("Servidor Diego"),
            EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        // 4. Delegado de callback
        CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
            FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGameInstanceFinal::OnCreateSessionComplete));

        // 5. Crear la sesión con el ID del jugador local
        const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
        {
            SessionInterface->CreateSession(
                *LocalPlayer->GetPreferredUniqueNetId(),
                NAME_GameSession,
                *SessionSettings);
        }
        else
        {
            // Fallback: crea con índice 0
            SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No se encontró ningún OnlineSubsystem activo"));
    }
}

void UGameInstanceFinal::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Sesión creada correctamente: %s"), *SessionName.ToString());

        // Equivalente al "Open Level (listen)" del nodo Blueprint
        UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("WaitngPlayers")), true, TEXT("listen"));

        // Si quieres imitar "Start After Create"
        SessionInterface->StartSession(NAME_GameSession);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Error al crear la sesión"));
    }
}
