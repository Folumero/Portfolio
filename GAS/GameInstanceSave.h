// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "GameInstanceSave.generated.h"

/**
 * 
 */
UCLASS()
class UTHUB_TFM_PROJECT_API UGameInstanceSave : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	bool FirstTimeExclude = true;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadWrite)
	TSet<FString> SelectedCardNames;

	UPROPERTY(BlueprintReadWrite)
	float SavedHealth;

	UPROPERTY(BlueprintReadWrite)
	float SavedDamage;

	UPROPERTY(BlueprintReadWrite)
	float SavedMaxHealth;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedLevel;

	UPROPERTY(BlueprintReadWrite)
	float SavedExperience;

	UPROPERTY(BlueprintReadWrite)
	FName LastMap;

	UPROPERTY(BlueprintReadWrite)
	float SoundMasterVolume=1;

	UPROPERTY(BlueprintReadWrite)
	int32 RoomsVisited = 0;

	//Abilities

	//Knight
	UPROPERTY(BlueprintReadWrite)
	float SpinRange = 300.f;

	UPROPERTY(BlueprintReadWrite)
	float JumpDamageModifier = 1.f;

	UPROPERTY(BlueprintReadWrite)
	int SwordsAmount=4;

	//Mage
	UPROPERTY(BlueprintReadWrite)
	float BombCircleRange = 1.f;

	UPROPERTY(BlueprintReadWrite)
	float MagicBallsAmount = 2.f;

	UPROPERTY(BlueprintReadWrite)
	float ThunderTimer = 0.7f;

	//Archer

	UPROPERTY(BlueprintReadWrite)
	float TripleArrowDamageMod = 1.f;

	UPROPERTY(BlueprintReadWrite)
	int AreaArrowAmount = 5;

	UPROPERTY(BlueprintReadWrite)
	float RainArrowDamageMod = 1.f;


	UPROPERTY(BlueprintReadWrite, Category="API")
	int ScoreEnemies = 0;
	
};
