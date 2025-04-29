// BattleField.cpp
#include "BattleField.h"
#include "Tile.h"
#include "CommanderOfFate.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"

ABattleField::ABattleField()
{
    PrimaryActorTick.bCanEverTick = false;

    static ConstructorHelpers::FClassFinder<ATile> TileBPClass(TEXT("/Game/Blueprints/BP_Tile"));
    if (TileBPClass.Succeeded())
    {
        TileClass = TileBPClass.Class;
    }
}

void ABattleField::BeginPlay()
{
    Super::BeginPlay();

    SpawnGrid();
    SpawnObstacles();

    // ❌ We REMOVE SpawnStartingUnits();
    // ✅ CommanderOfFate now handles Unit Placement after Toss!
}

void ABattleField::SpawnGrid()
{
    if (!TileClass) return;

    FVector Origin = FVector::ZeroVector - FVector((GridSize - 1) * TileSpacing / 2.0f, (GridSize - 1) * TileSpacing / 2.0f, 0.0f);

    for (int32 X = 0; X < GridSize; ++X)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            FVector SpawnLocation = Origin + FVector(X * TileSpacing, Y * TileSpacing, 0.0f);
            ATile* NewTile = GetWorld()->SpawnActor<ATile>(TileClass, SpawnLocation, FRotator::ZeroRotator);
            if (NewTile)
            {
                NewTile->SetCoordinates(X, Y);
                NewTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                AllTiles.Add(NewTile);
            }
        }
    }
}

void ABattleField::SpawnObstacles()
{
    if (AllTiles.Num() == 0) return;

    int32 TotalTiles = GridSize * GridSize;
    int32 ObstacleCount = FMath::Clamp(FMath::RoundToInt(TotalTiles * ObstaclePercentage), 0, TotalTiles);

    int32 MinX = GridSize * 0.15f;
    int32 MaxX = GridSize * 0.85f;
    int32 MinY = GridSize * 0.15f;
    int32 MaxY = GridSize * 0.85f;

    TArray<ETileType> ObstacleTypes = { ETileType::Mountain, ETileType::Tree1, ETileType::Tree2 };
    TMap<ETileType, int32> TypeCounts;
    int32 PerType = ObstacleCount / ObstacleTypes.Num();

    for (ETileType Type : ObstacleTypes)
        TypeCounts.Add(Type, 0);

    int32 Placed = 0;
    int32 Attempts = 0;

    while (Placed < ObstacleCount && Attempts < ObstacleCount * 10)
    {
        ATile* Tile = AllTiles[FMath::RandRange(0, AllTiles.Num() - 1)];
        if (!Tile || Tile->GetTileType() != ETileType::Empty)
        {
            ++Attempts;
            continue;
        }

        FVector2D Coord = Tile->GetCoordinates();
        if (Coord.X < MinX || Coord.X > MaxX || Coord.Y < MinY || Coord.Y > MaxY)
        {
            ++Attempts;
            continue;
        }

        for (ETileType Type : ObstacleTypes)
        {
            if (TypeCounts[Type] < PerType)
            {
                Tile->SetTileType(Type);
                ++TypeCounts[Type];
                ++Placed;
                break;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Obstacles: Mountain=%d, Tree1=%d, Tree2=%d"),
        TypeCounts[ETileType::Mountain], TypeCounts[ETileType::Tree1], TypeCounts[ETileType::Tree2]);
}
