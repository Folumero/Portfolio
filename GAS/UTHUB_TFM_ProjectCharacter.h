// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "CoreAttributeSet.h"
#include "GameplayTagContainer.h"
#include "ASC.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "UTHUB_TFM_ProjectCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterSwap);

UCLASS(config=Game)
class AUTHUB_TFM_ProjectCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagCustomInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//ABILITIES

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Interact;

	

public:
	AUTHUB_TFM_ProjectCharacter();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attack")
	bool IsAttacking;
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	

protected:
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	//ASC

private:

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UASC* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	class UGASDataComponent* GASDataComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> SampleEffect;;

	

	

public:

	UPROPERTY()
	UCoreAttributeSet* CoreAttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	FGameplayTag SelectedClassTag;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void AddTag(const FGameplayTag& InTag) override;
	virtual void RemoveTag(const FGameplayTag& InTag) override;

	UFUNCTION(CallInEditor, Category = "Test")
	void ApplyGameplayEffect();

	FORCEINLINE virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ActiveEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer GameplayStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanChangeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CharacterMaxHP;

	UFUNCTION()
	void ApplyCharacterChange();

	void RefreshInput();

	void CharacterSwapInteraction();

	UPROPERTY()
	FGameplayTag NewTag;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCharacterSwap OnCharacterSwap;


	//Abilities

	//Knight
	UPROPERTY(BlueprintReadWrite)
	float spinRange = 150.f;

	UPROPERTY(BlueprintReadWrite)
	float jumpDamageModifier = 1.f;

	UPROPERTY(BlueprintReadWrite)
	int swordsAmount = 4;

	//Mage
	UPROPERTY(BlueprintReadWrite)
	float bombCircleRange = 150.f;

	UPROPERTY(BlueprintReadWrite)
	float magicBallsAmount = 3.f;

	UPROPERTY(BlueprintReadWrite)
	float thunderTimer = 0.7f;

	//Archer

	UPROPERTY(BlueprintReadWrite)
	float tripleArrowDamageMod = 1.f;

	UPROPERTY(BlueprintReadWrite)
	int areaArrowAmount = 6;

	UPROPERTY(BlueprintReadWrite)
	float rainArrowDamageMod = 1.f;




protected:



	virtual void PreInitializeComponents() override;

	virtual void SetAttributeCallbacks();

	//Abilities Funcs

	void ExecuteAbility(const FInputActionInstance& InputActionInstance);



};

