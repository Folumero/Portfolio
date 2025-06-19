// Copyright Epic Games, Inc. All Rights Reserved.

#include "UTHUB_TFM_ProjectCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

//ASC

#include "GASDataComponent.h"
#include "ASC.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameInstanceSave.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUTHUB_TFM_ProjectCharacter

AUTHUB_TFM_ProjectCharacter::AUTHUB_TFM_ProjectCharacter()
{
	//GAS

	ASC = CreateDefaultSubobject<UASC>(TEXT("ASC"));

	CoreAttributeSet = CreateDefaultSubobject<UCoreAttributeSet>(TEXT("CoreAttr"));
	GASDataComponent = CreateDefaultSubobject<UGASDataComponent>(TEXT("GASData"));

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AUTHUB_TFM_ProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	SetAttributeCallbacks();
	if(ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
	}
	
	UGameInstanceSave* GI = Cast<UGameInstanceSave>(GetGameInstance());
	if(!GI->FirstTimeExclude)
	{
		SelectedClassTag = GI->SelectedClassTag;
		CoreAttributeSet->SetHealth(GI->SavedHealth);
		CoreAttributeSet->SetDamage(GI->SavedDamage);
		CharacterMaxHP = GI->SavedMaxHealth;


					//Abilities
		//Knight
		spinRange = GI->SpinRange;
		jumpDamageModifier = GI->JumpDamageModifier;
		//Mage
		bombCircleRange = GI->BombCircleRange;
		magicBallsAmount = GI->MagicBallsAmount;
		thunderTimer = GI->ThunderTimer;
		swordsAmount = GI->SwordsAmount;
		//Archer
		tripleArrowDamageMod = GI->TripleArrowDamageMod;
		areaArrowAmount = GI->AreaArrowAmount;
		rainArrowDamageMod = GI->RainArrowDamageMod;

	}
	else
	{
		GI->RoomsVisited = 0;
		CharacterMaxHP = CoreAttributeSet->GetHealth();
		GI->FirstTimeExclude = false;


					//Abilities
		//Knight
		spinRange = 300.f;
		jumpDamageModifier = 1.f;
		swordsAmount = 4;
		//Mage
		bombCircleRange = 1.f;
		magicBallsAmount = 2.f;
		thunderTimer = 0.7f;
		//Archer
		tripleArrowDamageMod = 1.f;
		areaArrowAmount = 5;
		rainArrowDamageMod = 1.f;


	}
	

	ApplyCharacterChange();
}


void AUTHUB_TFM_ProjectCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = GameplayStates;
}

void AUTHUB_TFM_ProjectCharacter::AddTag(const FGameplayTag& InTag)
{
	GameplayStates.AddTag(InTag);
}

void AUTHUB_TFM_ProjectCharacter::RemoveTag(const FGameplayTag& InTag)
{
	GameplayStates.RemoveTag(InTag);
}


void AUTHUB_TFM_ProjectCharacter::ApplyGameplayEffect()
{
	if(ASC)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SampleEffect, 1, EffectContext);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

 UAbilitySystemComponent* AUTHUB_TFM_ProjectCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

 void AUTHUB_TFM_ProjectCharacter::ApplyCharacterChange()
 {
	 if (SelectedClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("CharacterClass.Knight")))) 
	 {
		 //SkeletalMesh
		 USkeletalMesh* KnightMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Assets/Characters/Knight/Knight.Knight"));
		 if (KnightMesh)
		 {
			 GetMesh()->SetSkeletalMesh(KnightMesh);
		 }
		 //AnimClass
		 UClass* KnightAnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Alex/Anims/Anim_Knight.Anim_Knight_C"));
		 if (KnightAnimClass)
		 {
			 GetMesh()->SetAnimInstanceClass(KnightAnimClass);
		 }
		 //InputMapping
		 GASDataComponent->InputAbilityMapping = Cast<UInputAbilityMapping>(
			 StaticLoadObject(UInputAbilityMapping::StaticClass(), nullptr, TEXT("/Script/UTHUB_TFM_Project.InputAbilityMapping'/Game/Inputs/IAM_BasicActions.IAM_BasicActions'")));

	 }
	 else if (SelectedClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("CharacterClass.Mage")))) 
	 {
		 USkeletalMesh* MageMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Assets/Characters/Mage/Mage.Mage"));
		 if (MageMesh)
		 {
			 GetMesh()->SetSkeletalMesh(MageMesh);
		 }

		 UClass* MageAnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Alex/Anims/Anim_Mage.Anim_Mage_C"));
		 if (MageAnimClass)
		 {
			 GetMesh()->SetAnimInstanceClass(MageAnimClass);
		 }

		 GASDataComponent->InputAbilityMapping = Cast<UInputAbilityMapping>(
			 StaticLoadObject(UInputAbilityMapping::StaticClass(), nullptr, TEXT("/Script/UTHUB_TFM_Project.InputAbilityMapping'/Game/Inputs/IAM_Mage_BasicActions.IAM_Mage_BasicActions'")));
	 }

	 else if (SelectedClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("CharacterClass.Archer"))))
	 {
		 USkeletalMesh* RogueMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Assets/Characters/Archer/Rogue.Rogue"));
		 if (RogueMesh)
		 {
			 GetMesh()->SetSkeletalMesh(RogueMesh);
		 }
		 UClass* RogueAnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Alex/Anims/Anim_Rogue.Anim_Rogue_C"));
		 if (RogueAnimClass)
		 {
			 GetMesh()->SetAnimInstanceClass(RogueAnimClass);
		 }

		 GASDataComponent->InputAbilityMapping = Cast<UInputAbilityMapping>(
			 StaticLoadObject(UInputAbilityMapping::StaticClass(), nullptr, TEXT("/Script/UTHUB_TFM_Project.InputAbilityMapping'/Game/Inputs/IAM_ArcherActions.IAM_ArcherActions'")));
	 }

	 OnCharacterSwap.Broadcast();
	 RefreshInput();

 }

 void AUTHUB_TFM_ProjectCharacter::RefreshInput()
 {

	 if (!GASDataComponent || !GASDataComponent->InputAbilityMapping)
	 {
		 return;
	 }
	 UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	 if (!EnhancedInputComponent)
	 {
		 return;
	 }
	 EnhancedInputComponent->ClearActionBindings();

	 if (ASC)
	 {
		 TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
		 for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		 {
			 AbilitiesToRemove.Add(Spec.Handle);
		 }

		 for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToRemove)
		 {
			 ASC->ClearAbility(Handle);
		 }
	 }
	 EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::Move);
	 EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::Look);
	 EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::CharacterSwapInteraction);


	 for (auto [InputAction, AbilityClass] : GASDataComponent->InputAbilityMapping->Mappings)
	 {
		 if (InputAction && AbilityClass && ASC)
		 {
			 ASC->AddAbilityFromClass(AbilityClass);
			 EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &ThisClass::ExecuteAbility);
		 }
	 }

 }

void AUTHUB_TFM_ProjectCharacter::CharacterSwapInteraction()
{
	if(CanChangeClass)
	{
		SelectedClassTag = NewTag;

		ApplyCharacterChange();

	}

}


void AUTHUB_TFM_ProjectCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AUTHUB_TFM_ProjectCharacter::SetAttributeCallbacks()
{
	for(auto[Attribute, EffectorClass] : GASDataComponent->AttributeEffectors)
	{
		auto& Delegate = ASC->GetGameplayAttributeValueChangeDelegate(Attribute);

		UGameplayAttributeEffector* Effector = EffectorClass->GetDefaultObject<UGameplayAttributeEffector>();

		Delegate.AddUObject(Effector, &UGameplayAttributeEffector::ApplyAttributeEffector);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUTHUB_TFM_ProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::Look);

		//Abilities

		EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Triggered, this, &AUTHUB_TFM_ProjectCharacter::CharacterSwapInteraction);

		if(GASDataComponent)
		{
			
			for(auto [InputAction, AbilityClass] : GASDataComponent->InputAbilityMapping->Mappings)
			{
				if(InputAction)
				{
					if (AbilityClass)
					{
						if(ASC)
						{
							
							ASC->AddAbilityFromClass(AbilityClass);
							EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &ThisClass::ExecuteAbility);
							
						}
						
					}
				}
								
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AUTHUB_TFM_ProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !IsAttacking)
	{
		
		// Obtener la rotación de la cámara (sin Pitch ni Roll)
		FRotator CameraRotation = FollowCamera->GetComponentRotation();
		CameraRotation.Pitch = 0.0f;
		CameraRotation.Roll = 0.0f;

		// Obtener vectores de dirección a partir de la cámara
		FVector ForwardDirection = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);
		FVector RightDirection = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);

		// Calcular la dirección final del movimiento
		FVector MoveDirection = (ForwardDirection * MovementVector.Y) + (RightDirection * MovementVector.X);
		MoveDirection.Z = 0.0f; // Evitar movimientos en el eje Z

		// Rotar el personaje hacia la dirección de movimiento
		if (!MoveDirection.IsNearlyZero())
		{
			FRotator TargetRotation = MoveDirection.Rotation();
			TargetRotation.Pitch = 0.0f;
			TargetRotation.Roll = 0.0f;
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f));
		}

		// Aplicar el movimiento
		AddMovementInput(MoveDirection.GetSafeNormal());

	}
}

void AUTHUB_TFM_ProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUTHUB_TFM_ProjectCharacter::ExecuteAbility(const FInputActionInstance& InputActionInstance)
{
	
	if(GASDataComponent)
	{
		if(ASC)
		{
			const UInputAction* Action = InputActionInstance.GetSourceAction();
			if(Action)
			{
							
				ASC->TryActivateAbilityByClass(GASDataComponent->InputAbilityMapping->Mappings[Action]);
			}
		
		}
	}
}
