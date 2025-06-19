// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
void UAttributeComponent::HealPotion(float HealRegen)
{
	Health = FMath::Clamp(Health + HealRegen, 0.f, MaxHealth);
}

void UAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health-Damage, 0.f, MaxHealth);
}

void UAttributeComponent::ManaPotion(float ManaRegen)
{
	Mana = FMath::Clamp(Mana + ManaRegen, 0.f, MaxMana);
}

void UAttributeComponent::UseMana(float ManaCost)
{
	Mana = FMath::Clamp(Mana - ManaCost, 0.f, MaxMana);
}

float UAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}

float UAttributeComponent::GetManaPercent()
{
	return Mana / MaxMana;
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::AddSouls(int32 NumberOfSouls)
{
	Souls += NumberOfSouls;
}

void UAttributeComponent::UseSouls(int32 NumberOfSouls)
{
	if (NumberOfSouls > Souls) return;

	Souls -= NumberOfSouls;
}

void UAttributeComponent::AddGold(int32 AmountOfGold)
{
	Gold += AmountOfGold;
}


// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

void UAttributeComponent::RegenMana(float DeltaTime)
{
	Mana = FMath::Clamp(Mana + ManaRegenRate * DeltaTime, 0.f, MaxMana);
}

