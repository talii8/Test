// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleField.generated.h"

// Forward Declarations
class ATile;
class APlayerUnit;
class ACommanderOfFate;

UENUM(BlueprintType)
enum class EPlayerTurn : uint8
{
    HumanPlayer,
    AIPlayer
};

UCLASS()
class ALI_ADWS_API ABattleField : public AActor
{
    GENERATED_BODY()

public:
    ABattleField();

protected:
    virtual void BeginPlay() override;

private:
    void SpawnGrid();
    void SpawnObstacles();
    void SpawnStartingUnits();
    void StartPlacementPhase();

    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<ATile> TileClass;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float ObstaclePercentage = 0.20f;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> GreenSniperClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> GreenBrawlerClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> RedSniperClass;

    UPROPERTY(EditAnywhere, Category = "Units")
    TSubclassOf<APlayerUnit> RedBrawlerClass;

    static constexpr int32 GridSize = 25;
    static constexpr float TileSpacing = 110.0f;

    TArray<ATile*> AllTiles;
    TArray<ATile*> OccupiedTiles; // Track tiles where units are placed

    ACommanderOfFate* CommanderOfFate = nullptr;

    EPlayerTurn CurrentPlayerTurn;
};
