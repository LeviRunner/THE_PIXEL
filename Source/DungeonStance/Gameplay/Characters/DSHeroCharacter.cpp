#include "Gameplay/Characters/DSHeroCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Core/DSGameMode.h"
#include "Core/DSGameplayTags.h"
#include "Core/DSPlayerState.h"
#include "Gameplay/Abilities/DSAbilitySet.h"
#include "Gameplay/Abilities/DSAbilitySystemComponent.h"
#include "Gameplay/Attributes/DSAttributeSet.h"
#include "Gameplay/Characters/DSPlayerStateBridge.h"
#include "Gameplay/Characters/DSTargetLockComponent.h"
#include "Gameplay/Data/DSClassData.h"
#include "Gameplay/DSGameplayTagsGameplay.h"
#include "Gameplay/Effects/DSGameplayEffects.h"
#include "Gameplay/Stance/DSStanceComponent.h"

ADSHeroCharacter::ADSHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 720.f, 0.f);
	Movement->JumpZVelocity = 520.f; // ~140 cm de pulo
	Movement->MaxWalkSpeed = 500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 900.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	TargetLock = CreateDefaultSubobject<UDSTargetLockComponent>(TEXT("TargetLock"));
	StanceComponent = CreateDefaultSubobject<UDSStanceComponent>(TEXT("Stance"));

	PassiveEffects.Add(UDSGE_Regen::StaticClass());
	PassiveEffects.Add(UDSGE_StaminaRegen::StaticClass());
}

// ---------------------------------------------------------------------------
// GAS
// ---------------------------------------------------------------------------

void ADSHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilitySystemFromPlayerState();
}

void ADSHeroCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilitySystemFromPlayerState();
}

void ADSHeroCharacter::InitAbilitySystemFromPlayerState()
{
	ADSPlayerState* DSPlayerState = GetPlayerState<ADSPlayerState>();
	UDSAbilitySystemComponent* ASC = DSPlayerStateBridge::GetASC(DSPlayerState);
	if (!ASC)
	{
		return;
	}
	AbilitySystemComponent = ASC;
	AttributeSet = DSPlayerStateBridge::GetAttributeSet(DSPlayerState);
	ASC->InitAbilityActorInfo(DSPlayerState, this);

	if (HasAuthority())
	{
		InitializeServerState(DSPlayerState);
	}
	StanceComponent->InitializeForPlayerState(DSPlayerState);
	OnAbilitySystemInitialized();
}

void ADSHeroCharacter::InitializeServerState(ADSPlayerState* DSPlayerState)
{
	AbilitySystemComponent->SetReplicatedLooseTag(DSGameplayTags::Character_Hero, true);
	// Pawn novo (início da run ou renascimento): limpa estados de morte e reinicia atributos da classe.
	AbilitySystemComponent->SetReplicatedLooseTag(DSTags::State_Downed, false);
	AbilitySystemComponent->SetReplicatedLooseTag(DSTags::State_Dead, false);
	if (ClassData)
	{
		ClassData->ApplyBaseStats(AbilitySystemComponent, DSPlayerStateBridge::GetClassRank(DSPlayerState));
		// A vida máxima atual inclui as cartas; começa cheio.
		AbilitySystemComponent->SetNumericAttributeBase(UDSAttributeSet::GetHealthAttribute(),
			AbilitySystemComponent->GetNumericAttribute(UDSAttributeSet::GetMaxHealthAttribute()));
		for (const UDSAbilitySet* Set : ClassData->AbilitySets)
		{
			AbilitySystemComponent->GrantAbilitySet(Set, ClassData);
		}
	}
	ApplyPassiveEffects();
}

void ADSHeroCharacter::ApplyPassiveEffects()
{
	for (const TSubclassOf<UGameplayEffect>& EffectClass : PassiveEffects)
	{
		// O ASC sobrevive ao pawn: não duplica efeitos infinitos.
		if (EffectClass && AbilitySystemComponent->GetGameplayEffectCount(EffectClass, nullptr) == 0)
		{
			AbilitySystemComponent->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.f, AbilitySystemComponent->MakeEffectContext());
		}
	}
}

void ADSHeroCharacter::OnAbilitySystemInitialized()
{
	Super::OnAbilitySystemInitialized();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDSAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ADSHeroCharacter::OnHealthChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(DSTags::State_Downed, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	AbilitySystemComponent->RegisterGameplayTagEvent(DSTags::State_Downed, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ADSHeroCharacter::OnDownedTagChanged);
}

// ---------------------------------------------------------------------------
// Abatido / reviver
// ---------------------------------------------------------------------------

bool ADSHeroCharacter::IsDowned() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DSTags::State_Downed);
}

void ADSHeroCharacter::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude)
{
	// Herói não morre: fica abatido até a próxima onda (squad_match.ts) e o Core decide a derrota.
	if (HasAuthority())
	{
		SetDowned(true);
	}
}

void ADSHeroCharacter::SetDowned(bool bNewDowned)
{
	if (!AbilitySystemComponent || bDowned == bNewDowned)
	{
		return;
	}
	bDowned = bNewDowned;
	AbilitySystemComponent->SetReplicatedLooseTag(DSTags::State_Downed, bNewDowned);
	if (bNewDowned)
	{
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->ClearAbilityInput();
		if (TargetLock)
		{
			TargetLock->ClearLock();
		}
		if (ADSGameMode* GameMode = GetWorld()->GetAuthGameMode<ADSGameMode>())
		{
			GameMode->NotifyHeroDowned(this);
		}
	}
}

void ADSHeroCharacter::Revive(float HealthRatio)
{
	if (!HasAuthority() || !AbilitySystemComponent || !AttributeSet)
	{
		return;
	}
	const float Health = AttributeSet->GetMaxHealth() * FMath::Clamp(HealthRatio, 0.01f, 1.f);
	AbilitySystemComponent->SetNumericAttributeBase(UDSAttributeSet::GetHealthAttribute(), Health); // OnHealthChanged limpa o estado
}

void ADSHeroCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// Funciona também com o ReviveHero do Core (que só mexe na vida e nas tags soltas locais).
	if (HasAuthority() && bDowned && Data.NewValue > 0.f)
	{
		SetDowned(false);
	}
}

void ADSHeroCharacter::OnDownedTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	K2_OnDownedChanged(NewCount > 0);
	RefreshMovementSpeed();
}

void ADSHeroCharacter::HandleDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* Spec, float Magnitude)
{
	// Recurso ao sofrer dano: ganho × dano / 10 (BasePlayer.gd).
	if (HasAuthority() && ClassData && ClassData->Resource.GainOnDamaged > 0.f && AbilitySystemComponent)
	{
		AbilitySystemComponent->ApplyModToAttribute(UDSAttributeSet::GetResourceAttribute(), EGameplayModOp::Additive,
			ClassData->Resource.GainOnDamaged * Magnitude / 10.f);
	}
}

float ADSHeroCharacter::GetExtraSpeedMultiplier() const
{
	if (IsDowned())
	{
		return 0.f;
	}
	const bool bSprinting = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DSTags::State_Sprinting);
	return bSprinting ? SprintSpeedMultiplier : 1.f;
}

FVector ADSHeroCharacter::GetAimDirection() const
{
	if (const ADSCharacterBase* Target = TargetLock ? TargetLock->GetLockedTarget() : nullptr)
	{
		FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;
		if (!ToTarget.IsNearlyZero())
		{
			return ToTarget.GetSafeNormal();
		}
	}
	return Super::GetAimDirection();
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void ADSHeroCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer) : nullptr;
	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, MappingPriority);
	}
}

void ADSHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		return;
	}
	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADSHeroCharacter::Input_Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADSHeroCharacter::Input_Look);
	}
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (TargetLockAction)
	{
		Input->BindAction(TargetLockAction, ETriggerEvent::Started, this, &ADSHeroCharacter::Input_TargetLock);
	}
	if (TargetSwitchAction)
	{
		Input->BindAction(TargetSwitchAction, ETriggerEvent::Triggered, this, &ADSHeroCharacter::Input_TargetSwitch);
		Input->BindAction(TargetSwitchAction, ETriggerEvent::Completed, this, &ADSHeroCharacter::Input_TargetSwitchReleased);
	}
	for (const FDSInputAbilityBinding& Binding : AbilityInputBindings)
	{
		if (Binding.Action && Binding.InputTag.IsValid())
		{
			Input->BindAction(Binding.Action, ETriggerEvent::Started, this, &ADSHeroCharacter::Input_AbilityPressed, Binding.InputTag);
			Input->BindAction(Binding.Action, ETriggerEvent::Completed, this, &ADSHeroCharacter::Input_AbilityReleased, Binding.InputTag);
		}
	}
}

void ADSHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// O ADSPlayerController (Core) não chama ProcessAbilityInput; o pawn local processa aqui.
	if (AbilitySystemComponent && IsLocallyControlled())
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaSeconds, false);
	}
}

void ADSHeroCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller || IsDowned())
	{
		return;
	}
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void ADSHeroCharacter::Input_Look(const FInputActionValue& Value)
{
	if (TargetLock && TargetLock->IsLocked())
	{
		return; // câmera segue o alvo; o analógico direito troca de alvo
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ADSHeroCharacter::Input_TargetLock(const FInputActionValue& Value)
{
	if (TargetLock && !IsDowned())
	{
		TargetLock->ToggleLock();
	}
}

void ADSHeroCharacter::Input_TargetSwitch(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (!TargetLock || !TargetLock->IsLocked() || FMath::Abs(Axis) < 0.5f)
	{
		bTargetSwitchLatched = false;
		return;
	}
	if (!bTargetSwitchLatched)
	{
		bTargetSwitchLatched = true;
		TargetLock->SwitchTarget(Axis);
	}
}

void ADSHeroCharacter::Input_TargetSwitchReleased(const FInputActionValue& Value)
{
	bTargetSwitchLatched = false;
}

void ADSHeroCharacter::Input_AbilityPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagPressed(InputTag);
	}
}

void ADSHeroCharacter::Input_AbilityReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}
