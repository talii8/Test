// Tile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM(BlueprintType)
enum class ETileType : uint8
{
    Empty,
    Mountain,
    Tree1,
    Tree2
};

UCLASS()
class ALI_ADWS_API ATile : public AActor
{
    GENERATED_BODY()

public:
    ATile();

    void SetCoordinates(int32 X, int32 Y);
    FVector2D GetCoordinates() const;

    void SetTileType(ETileType Type);
    ETileType GetTileType() const;

    void HighlightTile(bool bHighlight, FColor HighlightColor = FColor::Green); // ✅ New for Movement Highlight

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* TileMesh;

    FVector2D GridCoordinates;
    ETileType TileType;

    void UpdateTileAppearance();

    UFUNCTION()
    void OnTileClicked(AActor* TouchedActor, FKey ButtonPressed); // ✅ For Tile Click Interaction
};
