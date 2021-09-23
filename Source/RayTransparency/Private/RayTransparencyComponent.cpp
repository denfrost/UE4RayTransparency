// Copyright 2021 Fly Dream Dev. All Rights Reserved. 

#include "RayTransparencyComponent.h"

#include "Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

URayTransparencyComponent::URayTransparencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	//PrimaryComponentTick.TickInterval = .1f;
	objectTypes.Add(ECC_WorldStatic);
}

void URayTransparencyComponent::SetSweepCapsule(float Radius, float HalfHeight)
{
	capsuleHalfHeight = HalfHeight;
	capsuleRadius = Radius;
}

void URayTransparencyComponent::SetFadeInOut(float InFadeInOutInterval)
{
	FadeInOutInterval = InFadeInOutInterval;
}

void URayTransparencyComponent::SetFadeRate(float InFadeRate)
{
	fadeRate = InFadeRate;
}

void URayTransparencyComponent::SetImmediatelyFade(float InImmediatelyFade)
{
	immediatelyFade = InImmediatelyFade;
}

void URayTransparencyComponent::SetCurrentTagActorsIgnore(FName CurrentTagIgnore)
{
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), CurrentTagIgnore, actorsIgnore);
}

void URayTransparencyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0))
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsLocalController())
		{
			GetOwner()->GetWorld()->GetTimerManager().SetTimer(CollectRayIntersectedMeshes_TimerHandle, this, &URayTransparencyComponent::CollectRayIntersectedMeshes, CollectRayTracedInterval, true, CollectRayTracedInterval);
			GetOwner()->GetWorld()->GetTimerManager().PauseTimer(CollectRayIntersectedMeshes_TimerHandle);

			GetOwner()->GetWorld()->GetTimerManager().SetTimer(MeshesFadeInOut_TimerHandle, this, &URayTransparencyComponent::MeshesFadeInOut, FadeInOutInterval, true, FadeInOutInterval);
			GetOwner()->GetWorld()->GetTimerManager().PauseTimer(CollectRayIntersectedMeshes_TimerHandle);

			SetActivate(bIsActivate);
			//GetOwner()->GetWorld()->GetTimerManager().SetTimer(objectWorkerTimer_Handle, this, &UAdvancedFadeObjectsPROComponent::FadeObjWorkerTimer, calcFadeInterval, true, calcFadeInterval);
		}

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TagActorsIgnore, actorsIgnore);
	FindPlayers();
}

void URayTransparencyComponent::CollectRayIntersectedMeshes()
{
	if (bIsEnabled)
	{
		// Have player or any camera?
		FindPlayers();

		int igNum = actorsIgnore.Num();
		//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Green, FString::Printf(TEXT("actorsIgnore[ %d ]"), igNum));
		for (AActor* currentActor_ : characterArray)
		{
			if (currentActor_)
			{
				const FVector traceStart_ = GEngine->GetFirstLocalPlayerController(GetOwner()->GetWorld())->PlayerCameraManager->GetCameraLocation();
				const FVector traceEnd_ = currentActor_->GetActorLocation();
				const FQuat actorQuat_ = currentActor_->GetActorQuat();

				if ((traceStart_ - traceEnd_).Size() < workDistance)
				{
					FCollisionQueryParams traceParams_(TEXT("RayTransparencyTrace"), bIsTraceComplex, GetOwner());

					traceParams_.AddIgnoredActors(actorsIgnore);
					traceParams_.bReturnPhysicalMaterial = false;
					// Not tracing complex uses the rough collision instead making tiny objects easier to select.
					traceParams_.bTraceComplex = bIsTraceComplex;

					TArray<FHitResult> hitArray_;
					TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes_;

					// Convert ECollisionChannel to ObjectType
					for (int i = 0; i < objectTypes.Num(); ++i)
					{
						traceObjectTypes_.Add(UEngineTypes::ConvertToObjectType(objectTypes[i].GetValue()));
					}


					GetOwner()->GetWorld()->SweepMultiByObjectType(hitArray_, traceStart_, traceEnd_, actorQuat_, traceObjectTypes_,
					                                               FCollisionShape::MakeCapsule(capsuleRadius, capsuleHalfHeight), traceParams_);

					
					for (int traceHitID_ = 0; traceHitID_ < hitArray_.Num(); ++traceHitID_)
					{
						if (hitArray_[traceHitID_].bBlockingHit && IsValid(hitArray_[traceHitID_].GetComponent()) && !fadeObjectsHit.Contains(hitArray_[traceHitID_].GetComponent()))
						{
							for (int hA = 0; hA < hitArray_.Num(); ++hA)
							{
								if (hitArray_[hA].bBlockingHit && IsValid(hitArray_[hA].GetComponent()) && !fadeObjectsHit.Contains(hitArray_[hA].GetComponent()))
								{
									fadeObjectsHit.AddUnique(hitArray_[hA].GetComponent());
								}
							}
						}
					} // End work trace
				}
			}
		}
	}

	// Make fade array after complete GetAllActorsOfClass loop
	for (int fO = 0; fO < fadeObjectsHit.Num(); ++fO)
	{
		// If not contains this component in fadeObjectsTemp
		if (!fadeObjectsTemp.Contains(fadeObjectsHit[fO]))
		{
			TArray<UMaterialInterface*> lBaseMaterials_;
			TArray<UMaterialInstanceDynamic*> lMidMaterials_;

			lBaseMaterials_.Empty();
			lMidMaterials_.Empty();

			fadeObjectsTemp.AddUnique(fadeObjectsHit[fO]);

			// For loop all materials ID in object
			for (int nM = 0; nM < fadeObjectsHit[fO]->GetNumMaterials(); ++nM)
			{
				lMidMaterials_.Add(UMaterialInstanceDynamic::Create(fadeMaterial, fadeObjectsHit[fO]));
				lBaseMaterials_.Add(fadeObjectsHit[fO]->GetMaterial(nM));

				// Set new material on object
				fadeObjectsHit[fO]->SetMaterial(nM, lMidMaterials_.Last());

			}
			// Create new fade object in array of objects to fade
			FTransparentObjectStruct newObject_;
			newObject_.NewElement(fadeObjectsHit[fO], lBaseMaterials_, lMidMaterials_,true);
			newObject_.fadeCurrent = immediatelyFade; //smooth fade fade
			// Add object to array
			fadeObjects.Add(newObject_);

			// Set collision on Primitive Component
			fadeObjectsHit[fO]->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		}
	}

	// Set hide to visible true if contains
	for (int fOT = 0; fOT < fadeObjectsTemp.Num(); ++fOT)
	{
		if (!fadeObjectsHit.Contains(fadeObjectsTemp[fOT]))
		{
			fadeObjects[fOT].SetHideOnly(false);
		}
	}


	int fOHNum = fadeObjectsHit.Num();
	//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("FadeObjectsHit[ %d ]"), fOHNum));
	// Clear array
	fadeObjectsHit.Empty();
}

void URayTransparencyComponent::MeshesFadeInOut()
{
	//FADE IN and OUT
	if (fadeObjects.Num() > 0)
	{
		for (int i = 0; i < fadeObjects.Num(); ++i)
		{
			//smooth Fade
			float adaptiveFade_;

			if (i == fadeObjects.Num())
			{
				adaptiveFade_ = nearObjectFade;
			}
			else
			{
				adaptiveFade_ = farObjectFade;
			}
			// For loop fadeMID array
			for (int t = 0; t < fadeObjects[i].fadeMID.Num(); ++t)
			{
				float targetF_;

				const float currentF = fadeObjects[i].fadeCurrent;

				if (fadeObjects[i].bToHide)
				{
					targetF_ = adaptiveFade_;
				}
				else
				{
					targetF_ = 1.0f;
				}

				const float newFade_ = FMath::FInterpConstantTo(currentF, targetF_, GetOwner()->GetWorld()->GetDeltaSeconds(), fadeRate);

				fadeObjects[i].fadeMID[t]->SetScalarParameterValue("Fade", newFade_);

				currentFade = newFade_;

				fadeObjects[i].SetFadeAndHide(newFade_, fadeObjects[i].bToHide);
			}
			// Set Fade end
			if ((!fadeObjects[i].bToHide) && (fadeObjects[i].primitiveComp) && (currentFade == 1.0f))
			{
				for (int bmi = 0; bmi < fadeObjects[i].baseMatInterface.Num(); ++bmi)
				{
					fadeObjects[i].primitiveComp->SetMaterial(bmi, fadeObjects[i].baseMatInterface[bmi]);
				}

				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("iBaseMats [ %d ] %s"), fadeObjects[i].baseMatInterface.Num(), *fadeObjects[i].primitiveComp->GetFullName()));

				fadeObjects[i].primitiveComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
				fadeObjects[i].Destroy();
				fadeObjects.RemoveAt(i);
				fadeObjectsTemp.RemoveAt(i);
			}
		}
		int fOTNum = fadeObjectsTemp.Num();
		//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Green, FString::Printf(TEXT("FadeObjectsTemp[ %d ]"), fOTNum));
		int fONum = fadeObjects.Num();
		//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Yellow, FString::Printf(TEXT("FadeObjects[ %d ]"), fONum));
	}
}

void URayTransparencyComponent::SetEnable(bool setEnable)
{
	bIsEnabled = setEnable;
}

void URayTransparencyComponent::SetActivate(bool setActivate)
{
	bIsActivate = setActivate;
	if (!bIsActivate)
	{
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(CollectRayIntersectedMeshes_TimerHandle);
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(MeshesFadeInOut_TimerHandle);
	}
	else
	{
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(CollectRayIntersectedMeshes_TimerHandle);
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(MeshesFadeInOut_TimerHandle);
	}
}

// Called every frame
void URayTransparencyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	//CollectRayIntersectedMeshes();
}

void URayTransparencyComponent::FindPlayers()
{
	// Find all actors from class.
	TArray<AActor*> allActorsArrTemp_;
	for (int classID_ = 0; classID_ < playerClassesArr.Num(); ++classID_)
	{
		if (playerClassesArr[classID_])
		{
			TArray<AActor*> allFoundActorsArr_;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), playerClassesArr[classID_], allFoundActorsArr_);

			allActorsArrTemp_.Append(allFoundActorsArr_);
		}
	}

	// Find only uniq actors.
	TArray<AActor*> allActorsArr_;
	for (int actorID_ = 0; actorID_ < allActorsArrTemp_.Num(); ++actorID_)
	{
		allActorsArr_.AddUnique(allActorsArrTemp_[actorID_]);
	}

	characterArray = allActorsArr_;
}