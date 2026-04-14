// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "LightningStrike.generated.h"

UCLASS()
class PROYECTO4_API ALightningStrike : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALightningStrike();

    // Niagara system for the lightning effect
    UPROPERTY(EditAnywhere, Category = "Effects")
    UNiagaraComponent* LightningEffect;

    UPROPERTY(EditAnywhere, Category = "Effects")
    UNiagaraSystem* LightningEffectSystem; // Niagara system for the lightning effect

    void ActivateLightning(const FVector& StartLocation, const FVector& EndLocation); // Function to activate the lightning effect
};
