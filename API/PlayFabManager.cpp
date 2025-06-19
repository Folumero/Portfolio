#include "PlayFabManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

APlayFabManager::APlayFabManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APlayFabManager::BeginPlay()
{
    Super::BeginPlay();

    TitleId = TEXT("198d3c");
    PlayerName = TEXT("TEST");  
    PlayerScore = 0;

    LoginToPlayFab();
}

void APlayFabManager::LoginToPlayFab()
{
    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
    RequestObj->SetStringField(TEXT("TitleId"), TitleId);
    RequestObj->SetStringField(TEXT("CustomId"), PlayerName);
    RequestObj->SetBoolField(TEXT("CreateAccount"), true);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(FString::Printf(TEXT("https://%s.playfabapi.com/Client/LoginWithCustomID"), *TitleId.ToLower()));
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &APlayFabManager::OnLoginResponse);
    Request->ProcessRequest();
}

void APlayFabManager::OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> Json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, Json))
        {
            SessionTicket = Json->GetObjectField("data")->GetStringField("SessionTicket");
            UE_LOG(LogTemp, Warning, TEXT("SessionTicket: %s"), *SessionTicket);

            SetDisplayName(PlayerName);

            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                FTimerDelegate::CreateUObject(this, &APlayFabManager::UpdateStatistics, PlayerScore),
                0.5f,
                false
            );
        }
    }
    else
    {
        if (Response.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Response->GetContentAsString());
        }

    }
}

void APlayFabManager::UpdateStatistics(int32 Score)
{
    if (Score <= 0)
    {
        return;
    }

    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> StatsArray;
    TSharedPtr<FJsonObject> StatObj = MakeShareable(new FJsonObject);

    StatObj->SetStringField("StatisticName", "highscore");
    StatObj->SetNumberField("Value", Score);
    StatsArray.Add(MakeShareable(new FJsonValueObject(StatObj)));
    RequestObj->SetArrayField("Statistics", StatsArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    FString Url = FString::Printf(TEXT("https://%s.playfabapi.com/Client/UpdatePlayerStatistics"), *TitleId.ToLower());

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("X-Authorization"), SessionTicket);
    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr, FHttpResponsePtr Response, bool bSuccess)
        {
            if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
            {
                UE_LOG(LogTemp, Warning, TEXT("Send stats:\n%s"), *Response->GetContentAsString());
            }

        });
    Request->ProcessRequest();
}

void APlayFabManager::SetDisplayName(FString DisplayName)
{
    if (SessionTicket.IsEmpty())
    {
        return;
    }

    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
    RequestObj->SetStringField(TEXT("DisplayName"), DisplayName);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    FString Url = FString::Printf(TEXT("https://%s.playfabapi.com/Client/UpdateUserTitleDisplayName"), *TitleId.ToLower());

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("X-Authorization"), SessionTicket);
    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr, FHttpResponsePtr Response, bool bSuccess)
        {
            if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
            {
                UE_LOG(LogTemp, Warning, TEXT("DisplayName good."));
            }
        });
    Request->ProcessRequest();
}
void APlayFabManager::GetTopLeaderboard()
{

    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
    RequestObj->SetStringField(TEXT("StatisticName"), TEXT("highscore"));
    RequestObj->SetNumberField(TEXT("StartPosition"), 0);
    RequestObj->SetNumberField(TEXT("MaxResultsCount"), 10);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    FString Url = FString::Printf(TEXT("https://%s.playfabapi.com/Client/GetLeaderboard"), *TitleId.ToLower());

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("X-Authorization"), SessionTicket);
    Request->SetContentAsString(OutputString);

    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSuccess)
        {
            if (!bSuccess || !Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Error leaderboard: %s"),
                    Response.IsValid() ? *Response->GetContentAsString() : TEXT("No response"));
                return;
            }

            if (Response->GetResponseCode() != 200)
            {
                return;
            }

            TSharedPtr<FJsonObject> Json;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (FJsonSerializer::Deserialize(Reader, Json))
            {
                const TArray<TSharedPtr<FJsonValue>> Entries = Json->GetObjectField("data")->GetArrayField("Leaderboard");
                LeaderboardData.Empty();

                for (const TSharedPtr<FJsonValue>& EntryValue : Entries)
                {
                    const TSharedPtr<FJsonObject> Entry = EntryValue->AsObject();
                    FLeaderboardEntry NewEntry;
                    NewEntry.DisplayName = Entry->GetStringField("DisplayName");
                    NewEntry.Score = Entry->GetIntegerField("StatValue");
                    NewEntry.Position = Entry->GetIntegerField("Position");
                    LeaderboardData.Add(NewEntry);
                }

               
            }

        });

    Request->ProcessRequest();
}
