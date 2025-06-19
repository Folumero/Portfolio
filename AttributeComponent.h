// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROYECTO4_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenMana(float DeltaTime);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Attributes")
	int32 Souls;
		
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Mana;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxMana;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold;


	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DodgeCost = 14.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float ManaRegenRate = 8.f;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Actor Attributes")
	void ReceiveDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Actor Attributes")
	void HealPotion(float HealRegen);

	UFUNCTION(BlueprintCallable, Category = "Actor Attributes")
	void ManaPotion(float ManaRegen);

	UFUNCTION(BlueprintCallable, Category = "Actor Attributes")
	void UseMana(float ManaCost);

	UFUNCTION(BlueprintPure, Category = "Actor Attributes")
	float GetHealthPercent();

	UFUNCTION(BlueprintPure, Category = "Actor Attributes")
	float GetManaPercent();

	UFUNCTION(BlueprintPure, Category = "Actor Attributes")
	bool IsAlive();

	UFUNCTION(BlueprintCallable, Category = "Actor Attributes")
	void AddSouls(int32 NumberOfSouls);

	UFUNCTION(BlueprintCallable, Category = "Character|Currency")
	void UseSouls(int32 NumberOfSouls);

	UFUNCTION(BlueprintCallable, Category = "Character|Currency")
	void AddGold(int32 AmountOfGold);

	FORCEINLINE void SetGold(int32 NumberOfGold) { Gold = NumberOfGold; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE float GetMana() const { return Mana; }
};
