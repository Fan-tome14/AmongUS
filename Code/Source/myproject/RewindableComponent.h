#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "RewindableComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API URewindableComponent : public UCapsuleComponent
{
    GENERATED_BODY()

public:
    URewindableComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};