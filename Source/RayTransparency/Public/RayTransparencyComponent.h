// Copyright 2021 Fly Dream Dev. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInterface.h"

#include "RayTransparencyComponent.generated.h"

USTRUCT(BlueprintType)
struct FTransparentObjectStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UPrimitiveComponent* primitiveComp;

	UPROPERTY()
	TArray<UMaterialInterface*> baseMatInterface;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> fadeMID;

	//smooth fade fade
	UPROPERTY()
	float fadeCurrent;

	UPROPERTY()
	bool bToHide;

	void NewElement(UPrimitiveComponent* newComponent, TArray<UMaterialInterface*> newBaseMat,
	                TArray<UMaterialInstanceDynamic*> newMID, bool bHide)
	{
		primitiveComp = newComponent;
		baseMatInterface = newBaseMat;
		fadeMID = newMID;
		bToHide = bHide;
	}

	void SetHideOnly(bool hide)
	{
		bToHide = hide;
	}

	void SetFadeAndHide(float newFade, bool newHide)
	{
		fadeCurrent = newFade;
		bToHide = newHide;
	}

	//For Destroy
	void Destroy()
	{
		primitiveComp = nullptr;
	}

	//Constructor
	FTransparentObjectStruct()
	{
		primitiveComp = nullptr;
		bToHide = true;
	}
};

UCLASS(Blueprintable)
class RAYTRANSPARENCY_API URayTransparencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URayTransparencyComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/////////FADE SECTION

	//FADE: delay outfade in time
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float FadeInOutInterval = 0.05f;
	//FADE: Rate fade increment
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float fadeRate = 0.5f;
	//FADE: Instance fade
	UPROPERTY(EditDefaultsOnly, Category = "RayTransparency Parameters")
	float immediatelyFade = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "RayTransparency Functions")
	void SetFadeInOut(float InFadeInOutInterval);
	UFUNCTION(BlueprintCallable, Category = "RayTransparency Functions")
	void SetFadeRate(float InfadeRate);
	UFUNCTION(BlueprintCallable, Category = "RayTransparency Functions")
	void SetImmediatelyFade(float InImmediatelyFade);

	//Control Ray Sweep sizes
	UFUNCTION(BlueprintCallable, Category = "RayTransparency Functions")
	void SetSweepCapsule(float radius, float HalfHeight);

	// Trace object size
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float capsuleHalfHeight = 10.0f;//88.0f;
	// Trace object size
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float capsuleRadius = 10.0f; // 34.0f;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Check objects between camera manager and character and add to array for fade
	void CollectRayIntersectedMeshes();

	void MeshesFadeInOut();

	void FindPlayers();

	// Enable or disable fade object worker
	UFUNCTION(BlueprintCallable, Category = "RayTransparency")
	void SetEnable(bool setEnable);

	// Pause or unpause fade object worker
	UFUNCTION(BlueprintCallable, Category = "RayTransparency")
	void SetActivate(bool setActivate);

	TEnumAsByte<ETraceTypeQuery> myTraceType;

	// Translucent material for fade object
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	UMaterialInstance* fadeMaterial;

	// Enable or disable fade object worker
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	bool bIsEnabled = true;

	// Pause or unpause fade object worker
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	bool bIsActivate = true;

	// This can reduce performance.
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	bool bIsTraceComplex = true; // Use like a Default

	// Timer intervals
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float CollectRayTracedInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float findPlayersInterval = 1.f;
	//Distance
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float workDistance = 1950000.0f;

	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	TArray<TSubclassOf<AActor>> playerClassesArr;

	// Check trace block by this
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	TArray<TEnumAsByte<ECollisionChannel>> objectTypes;

	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float nearObjectFade = 0.3;
	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "RayTransparency Parameters")
	float farObjectFade = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RayTransparency Parameters")
	TArray<AActor*> actorsIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RayTransparency Parameters")
	FName TagActorsIgnore = FName(TEXT("TraceIgnoreActor"));
	///
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FTransparentObjectStruct> fadeObjects;

	// Now ID
	int32 fadeNowID;

	// Primitive components temp variable
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UPrimitiveComponent*> fadeObjectsTemp;

	// Primitive components temp variable
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UPrimitiveComponent*> fadeObjectsHit;
	// Temp variable
	float currentFade;

private:

	// Some worker timer
	FTimerHandle CollectRayIntersectedMeshes_TimerHandle;
	FTimerHandle MeshesFadeInOut_TimerHandle;
	//FTimerHandle FindPlayers_TimerHandle;
	

	// All characters array (maybe you control ( > 1 ) characters)
	UPROPERTY()
	TArray<AActor*> characterArray;

};
