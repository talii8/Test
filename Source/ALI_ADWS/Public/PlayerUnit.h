#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerUnit.generated.h"

UENUM(BlueprintType)
enum class EUnitTeam : uint8
{
    Team_Red,
    Team_Green
};

UENUM(BlueprintType)
enum class EUnitType : uint8
{
    Sniper,
    Brawler
};

UCLASS()
class ALI_ADWS_API APlayerUnit : public AActor
{
    GENERATED_BODY()

public:
    APlayerUnit();

    void InitializeUnit(EUnitTeam InTeam, EUnitType InType);
    EUnitTeam GetTeam() const { return Team; }
    EUnitType GetUnitType() const { return UnitType; }
    int32 GetMovementRange() const { return MovementRange; }
    bool IsSelected() const { return bIsSelected; }

    void SelectUnit();
    void DeselectUnit();
    void FindReachableTiles();
    const TArray<FVector2D>& GetReachableTiles() const { return ReachableTiles; }

protected:
    virtual void BeginPlay() override;
    void ApplyTeamMaterial();

    UPROPERTY(VisibleAnywhere, Category = "Unit")
    UStaticMeshComponent* Mesh;

    UFUNCTION()
    void OnUnitClicked(AActor* TouchedActor, FKey ButtonPressed);

private:
    EUnitTeam Team;
    EUnitType UnitType;
    int32 MaxHP;
    int32 CurrentHP;
    int32 MovementRange;
    int32 AttackRange;

    bool bIsSelected = false;
    TArray<FVector2D> ReachableTiles;
};

