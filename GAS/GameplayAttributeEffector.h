// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTasksClasses.h"
#include "UObject/NoExportTypes.h"
#include "GameplayAttributeEffector.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class UTHUB_TFM_PROJECT_API UGameplayAttributeEffector : public UObject
{
	GENERATED_BODY()

public:
	void ApplyAttributeEffector(const FOnAttributeChangeData& InChangeData);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveApplyAttributeEffector(AActor* InActor, float InValue, float InOldValue);
	
};
