// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WavesManager.generated.h"

UCLASS()
class UTHUB_TFM_PROJECT_API AWavesManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWavesManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> EnemiesToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* DoorToOpen;
	
	UFUNCTION(BlueprintCallable)
	void openDoor();

	UPROPERTY(EditAnywhere, Category = "Doors")
	USoundBase* DoorSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RemainingEnemies;


};
