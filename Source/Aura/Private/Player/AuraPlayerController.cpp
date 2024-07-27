// Copyright MUGK Corp


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);

	Subsystem->AddMappingContext(AuraContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn *ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if (!CursorHit.bBlockingHit)
	{
		return;
	}

	LastActor = CurrentActor;
	CurrentActor = CursorHit.GetActor();

	/**
	 * A. LastActor is null && CurrentActor is null
	 *    - Do nothing
	 * B. LastActor is null && CurrentActor is valid
	 *    - Call Highlight on CurrentActor
	 * C. LastActor is valid && CurrentActor is null
	 *    - Call UnHighlight on LastActor
	 * D. LastActor is valid && CurrentActor is valid, but LastActor != CurrentActor
	 *    - Call UnHighlight on LastActor
	 *    - Call Highlight on CurrentActor
	 * E. LastActor is valid && CurrentActor is valid, but LastActor == CurrentActor
	 *    - Do nothing
	 */
	if (LastActor == nullptr)
	{
		if (CurrentActor != nullptr) // B
		{
			CurrentActor->HighlightActor();
		}

		// A
	}
	else
	{
		if (CurrentActor == nullptr) // C
		{
			LastActor->UnHighlightActor();
		}
		else if (CurrentActor != LastActor) // D
		{
			LastActor->UnHighlightActor();
			CurrentActor->HighlightActor();
		}

		// E
	}
}
