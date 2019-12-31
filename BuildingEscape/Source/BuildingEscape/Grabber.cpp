// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabber.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/InputComponent.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }

	///if physics handle is attached
	if (PhysicsHandle->GrabbedComponent) {
		///Move the object we are holding
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}
}

void UGrabber::Grab() {
	//UE_LOG(LogTemp, Warning, TEXT("Grab pressed"));

	///LINE TRACE and see if we reached any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInSearch();
	auto ComponentToGrab = HitResult.GetComponent(); /// get the mesh
	auto ActorHit = HitResult.GetActor();

	///If we hit something then attach physics handle
	if (ActorHit) {
		if (!PhysicsHandle) { return; }

		PhysicsHandle->GrabComponent(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation(), true);
	}

}

void UGrabber::Release() {
	//UE_LOG(LogTemp, Warning, TEXT("Grab released"));

	if (!PhysicsHandle) { return; }

	///Release physics handle
	PhysicsHandle->ReleaseComponent();
}

/// Get the physics handle component attach to the owner
void UGrabber::FindPhysicsHandleComponent() {
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	if (!PhysicsHandle) {
		UE_LOG(LogTemp, Error, TEXT("%s missing PhysicsHandle component"), *GetOwner()->GetName());
	}
}

/// Get the input component attach to the owner (Only appears at run tine)
void UGrabber::SetupInputComponent() {
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();

	if (InputComponent) {
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("%s missing InputComponent component"), *GetOwner()->GetName());
	}
}

FVector UGrabber::GetReachLineStart()
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	/// Get player view point
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	/*UE_LOG(LogTemp, Warning, TEXT("Player location is %s and rotation %s"), *PlayerViewPointLocation.ToString(), *PlayerViewPointRotation.ToString());*/

	return PlayerViewPointLocation;
}

FVector UGrabber::GetReachLineEnd()
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	/// Get player view point
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	/*UE_LOG(LogTemp, Warning, TEXT("Player location is %s and rotation %s"), *PlayerViewPointLocation.ToString(), *PlayerViewPointRotation.ToString());*/

	return PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;
}

FHitResult UGrabber::GetFirstPhysicsBodyInSearch()
{
	///Draw Debug line
	//DrawDebugLine(GetWorld(), PlayerViewPointLocation, LineTraceEnd, FColor(255, 0, 0), false, 0.0f, 0.0f, 10.0f);

	///Line trace (ray cast) out to reach destination
	FHitResult HitResult;
	///Setup query parameter
	FCollisionQueryParams TraceParameter(FName(TEXT("")), false, GetOwner());

	GetWorld()->LineTraceSingleByObjectType(OUT HitResult, GetReachLineStart(), GetReachLineEnd(), FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), TraceParameter);

	return HitResult;
}