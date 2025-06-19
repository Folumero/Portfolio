// Fill out your copyright notice in the Description page of Project Settings.


#include "WavesManager.h"
#include "Kismet/GameplayStatics.h"
#include "Door.h"

// Sets default values
AWavesManager::AWavesManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWavesManager::BeginPlay()
{
	Super::BeginPlay();

}

void AWavesManager::openDoor()
{
	//RemainingEnemies--;
	if (RemainingEnemies <=0 && DoorToOpen)
	{
		
		ADoor* DoorScript = Cast<ADoor>(DoorToOpen);
		if (DoorScript) { DoorScript->IsOpen = true; UGameplayStatics::PlaySoundAtLocation(this, DoorSound, GetActorLocation());
		}

		//End in blueprint
		
	}
}

void AWavesManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

