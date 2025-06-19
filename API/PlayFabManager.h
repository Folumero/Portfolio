#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "PlayFabManager.generated.h"

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite)
    int32 Score;

    UPROPERTY(BlueprintReadWrite)
    int32 Position;
};
UCLASS()
class UTHUB_TFM_PROJECT_API APlayFabManager : public AActor
{
    GENERATED_BODY()

public:
    APlayFabManager();

public:
    UPROPERTY(BlueprintReadOnly)
    TArray<FLeaderboardEntry> LeaderboardData;

    UFUNCTION(BlueprintCallable)
    void GetTopLeaderboard();

    UFUNCTION(BlueprintCallable)
    const TArray<FLeaderboardEntry>& GetLeaderboardData() const { return LeaderboardData; }

protected:
    virtual void BeginPlay() override;


    void OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);





    FString SessionTicket;
    FString TitleId = TEXT("198D3C");

public:

    UFUNCTION(BlueprintCallable, Category = "PlayFab")
    void LoginToPlayFab();

    UFUNCTION(BlueprintCallable, Category = "PlayFab")
    void UpdateStatistics(int32 Score);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayFab")
    FString PlayerName = "TEST";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayFab")
    int32 PlayerScore = 0;

    void SetDisplayName(FString DisplayName);
};
