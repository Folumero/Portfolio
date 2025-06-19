// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC.h"
#include "CoreAttributeSet.h"
#include "GASDataComponent.h"
#include "NativeGameplayTags.h"

UASC::UASC()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UASC::ApplyEffectFromClass(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(EffectClass, 1, EffectContext);

	ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UASC::RemoveEffect(FActiveGameplayEffectHandle EffectHandle)
{
	if (EffectHandle.IsValid())
	{
		RemoveActiveGameplayEffect(EffectHandle);
	}
}

void UASC::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo(GetOwner(), GetOwner());
}

void UASC::InitializeAttributesFromEffect()
{
	if (UGASDataComponent* DataComponent = GetOwner()->FindComponentByClass<UGASDataComponent>())
	{
		for (const auto& EffectClass : DataComponent->AttributeInitializers)
		{
			ApplyEffectFromClass(EffectClass);
		}
	}
}

void UASC::AddAbilityFromClass(TSubclassOf<UGameplayAbility>& InGameplayAnilityClass)
{
	FGameplayAbilitySpec Spec(InGameplayAnilityClass, 1, 0);

	GiveAbility(Spec);
}

void UASC::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	InitializeAttributesFromEffect();

	UGASDataComponent* DataComponent = GetOwner()->FindComponentByClass<UGASDataComponent>();
	if (DataComponent)
	{
		
		

	}

}

void UASC::InitializeAttribute(AActor* InOwnerActor)
{

	if (UCoreAttributeSet* CoreAttributes = GetAttributeSetFrowmOwner<UCoreAttributeSet>())
	{
		if (UGASDataComponent* DataComponent = InOwnerActor->FindComponentByClass<UGASDataComponent>())
		{
			if (DataComponent->DT_CoreStats)
			{


				auto InitializeAttrribute = [this, CoreAttributes](const FName& RowName, const FGameplayCoreAttribute& Row)
					{


						float NewValue = Row.AttributeBaeValue;

						Row.Attribute.SetNumericValueChecked(NewValue, CoreAttributes);

					};
				DataComponent->DT_CoreStats->ForeachRow<FGameplayCoreAttribute>(TEXT(""), MoveTemp(InitializeAttrribute));
			}
		}
	}
}

void UASC::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
