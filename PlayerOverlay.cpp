// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
        HealthProgressBar->SetPercent(Percent);
        ActualHealth = Percent;
        if (CurrentEnergyLifePercent < ActualHealth) {
            CurrentEnergyLifePercent = ActualHealth;
            if (shouldClearHealthTimer)
            {
                GetWorld()->GetTimerManager().ClearTimer(EnergyLifeTimerHandle);
            }
            if (EnergyLifeProgressBar) {
                EnergyLifeProgressBar->SetPercent(CurrentEnergyLifePercent);
            }
        }
	}
}

void UPlayerOverlay::SetManaBarPercent(float Percent)
{
    if (ManaProgressBar)
    {
        ManaProgressBar->SetPercent(Percent);
        ActualMana = Percent;
        if (CurrentEnergyManaPercent < ActualMana) {
            CurrentEnergyManaPercent = ActualMana;
            if (shouldClearManaTimer)
            {
                GetWorld()->GetTimerManager().ClearTimer(EnergyManaTimerHandle);
            }
            if (EnergyManaProgressBar)
            {
                EnergyManaProgressBar->SetPercent(CurrentEnergyManaPercent);
            }
        }
    }
}

void UPlayerOverlay::SetHealthEnergyBarPercent(float Percent) {
    TargetEnergyLifePercent = Percent;
    if (!GetWorld()->GetTimerManager().IsTimerActive(EnergyLifeTimerHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(EnergyLifeTimerHandle, this, &UPlayerOverlay::UpdateEnergyLifeBar, 0.05f, true);
    }
}

void UPlayerOverlay::SetManaEnergyBarPercent(float Percent) {
    TargetEnergyManaPercent = Percent;
    if (!GetWorld()->GetTimerManager().IsTimerActive(EnergyManaTimerHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(EnergyManaTimerHandle, this, &UPlayerOverlay::UpdateEnergyManaBar, 0.05f, true);
    }
}


void UPlayerOverlay::UpdateEnergyLifeBar()
{
    const float Increment = 0.01f;
    shouldClearHealthTimer = false;

    if (CurrentEnergyLifePercent > TargetEnergyLifePercent)
    {
        CurrentEnergyLifePercent -= Increment;
        if (CurrentEnergyLifePercent < ActualHealth) {
            CurrentEnergyLifePercent = ActualHealth;
            shouldClearHealthTimer = true;
        }

    }

    if (EnergyLifeProgressBar)
    {
        EnergyLifeProgressBar->SetPercent(CurrentEnergyLifePercent);
    }

    if (shouldClearHealthTimer)
    {
        GetWorld()->GetTimerManager().ClearTimer(EnergyLifeTimerHandle);
    }
}

void UPlayerOverlay::UpdateEnergyManaBar()
{
    const float Increment = 0.01f;  
    shouldClearManaTimer = false;

    if (CurrentEnergyManaPercent > TargetEnergyManaPercent)
    {
        CurrentEnergyManaPercent -= Increment;
        if (CurrentEnergyManaPercent < ActualMana) {
            CurrentEnergyManaPercent = TargetEnergyManaPercent;
            shouldClearManaTimer = true;
        }
    }

    if (EnergyManaProgressBar)
    {
        EnergyManaProgressBar->SetPercent(CurrentEnergyManaPercent);
    }

    if (shouldClearManaTimer)
    {
        GetWorld()->GetTimerManager().ClearTimer(EnergyManaTimerHandle);
    }
}

void UPlayerOverlay::SetGold(int32 Gold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Gold)));
	}
}

void UPlayerOverlay::SetSouls(int32 Souls)
{
	if (SoulsText)
	{
		SoulsText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Souls)));
	}
}
