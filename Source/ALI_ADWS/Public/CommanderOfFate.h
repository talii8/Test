#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerUnit.h"
#include "CommanderOfFate.generated.h"

UCLASS()
class ALI_ADWS_API ACommanderOfFate : public AActor
{
    GENERATED_BODY()

public:
    ACommanderOfFate();

protected:
    virtual void BeginPlay() override;

public:
    void StartPlacementPhase();
    void HandlePlacement(FVector2D TargetTileCoord);
    void HandleTileClicked(FVector2D TileCoord);

    void SetSelectedUnit(APlayerUnit* NewSelectedUnit);
    void HighlightMovementTiles();
    void ClearHighlightedTiles();

    void ExecuteAITurn();  // 🔥 AI Turn Execution
    FVector2D GetUnitGridCoord(APlayerUnit* Unit) const; // 🔥 Helper

private:
    bool bPlacementPhaseActive = false;
    bool bTossWinnerIsHuman = false;
    int32 PlacementStep = 0;

    APlayerUnit* SelectedUnit = nullptr;
    TArray<class ATile*> ReachableTilesForMovement;

    bool TryPlaceUnitAt(FVector2D Coord, bool bIsHuman, EUnitType UnitType);
    class ATile* FindRandomEmptyTile();
    void StartGameTurns();
    void EndPlacementPhase();
    bool IsCurrentTurnHuman() const;
    bool IsPlacementPhaseComplete() const;
    EUnitType GetUnitTypeForCurrentPlacement() const;
    void MoveSelectedUnitTo(FVector2D TileCoord);

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> GreenSniperClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> GreenBrawlerClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> RedSniperClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> RedBrawlerClass;
};
