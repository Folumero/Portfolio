// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PROYECTO4_API UPlayerOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetHealthBarPercent(float Percent);
	void UpdateEnergyLifeBar();
	void UpdateEnergyManaBar();
	void SetManaBarPercent(float Percent);
	void SetHealthEnergyBarPercent(float Percent);
	void SetManaEnergyBarPercent(float Percent);
	void SetGold(int32 Gold);
	void SetSouls(int32 Souls);

private:
	bool shouldClearManaTimer = false;
	bool shouldClearHealthTimer = false;
	float ActualHealth = 0.0f;
	float ActualMana = 0.0f;
	float CurrentEnergyLifePercent = 1.0f;
	float TargetEnergyLifePercent = 0.0f;
	FTimerHandle EnergyLifeTimerHandle;

	float CurrentEnergyManaPercent = 1.0f;
	float TargetEnergyManaPercent = 0.0f;
	FTimerHandle EnergyManaTimerHandle;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnergyLifeProgressBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnergyManaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ManaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SoulsText;
};
