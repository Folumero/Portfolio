// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAttributeEffector.h"
#include "CoreAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"

void UGameplayAttributeEffector::ApplyAttributeEffector(const FOnAttributeChangeData& InChangeData)
{
	ReceiveApplyAttributeEffector(InChangeData.GEModData->Target.GetOwner(), InChangeData.NewValue, InChangeData.OldValue);
}
