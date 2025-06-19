// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/LightningStrike.h"
#include "CoreMinimal.h"
#include "UObject/NameTypes.h"
#include "DrawDebugHelpers.h"

ALightningStrike::ALightningStrike()
{
    PrimaryActorTick.bCanEverTick = false;

    LightningEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightningEffect"));
    RootComponent = LightningEffect; 
}

void ALightningStrike::ActivateLightning(const FVector& StartLocation, const FVector& EndLocation)
{
    if (LightningEffectSystem)
    {
        LightningEffect->SetAsset(LightningEffectSystem);
        LightningEffect->SetNiagaraVariableVec3(FName("PosicionFinal").ToString(), EndLocation); 
        LightningEffect->Activate();
    }
}