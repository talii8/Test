// PlayerUnit.cpp
#include "PlayerUnit.h"
#include "Tile.h"
#include "Kismet/GameplayStatics.h"
#include "CommanderOfFate.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

APlayerUnit::APlayerUnit()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = Mesh;
}

void APlayerUnit::BeginPlay()
{
    Super::BeginPlay();
    ApplyTeamMaterial();
    OnClicked.AddDynamic(this, &APlayerUnit::OnUnitClicked);
}

void APlayerUnit::InitializeUnit(EUnitTeam InTeam, EUnitType InType)
{
    Team = InTeam;
    UnitType = InType;

    switch (UnitType)
    {
    case EUnitType::Sniper:
        MaxHP = 20;
        MovementRange = 3;
        AttackRange = 10;
        break;
    case EUnitType::Brawler:
        MaxHP = 40;
        MovementRange = 6;
        AttackRange = 1;
        break;
    }

    CurrentHP = MaxHP;
}

void APlayerUnit::ApplyTeamMaterial()
{
    FString MaterialPath;

    if (UnitType == EUnitType::Sniper)
    {
        MaterialPath = (Team == EUnitTeam::Team_Green)
            ? TEXT("/Game/Textures/M_Sniper_Green")
            : TEXT("/Game/Textures/M_Sniper_Red");
    }
    else if (UnitType == EUnitType::Brawler)
    {
        MaterialPath = (Team == EUnitTeam::Team_Green)
            ? TEXT("/Game/Textures/M_Brawler_Green")
            : TEXT("/Game/Textures/M_Brawler_Red");
    }

    UMaterialInterface* Material = Cast<UMaterialInterface>(
        StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath)
    );

    if (Material)
    {
        Mesh->SetMaterial(0, Material);
    }
}

void APlayerUnit::OnUnitClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (Team == EUnitTeam::Team_Green)
    {
        SelectUnit();
        ACommanderOfFate* Commander = Cast<ACommanderOfFate>(UGameplayStatics::GetActorOfClass(GetWorld(), ACommanderOfFate::StaticClass()));
        if (Commander)
        {
            Commander->SetSelectedUnit(this);
        }
    }
}

void APlayerUnit::SelectUnit()
{
    if (!bIsSelected)
    {
        bIsSelected = true;
        Mesh->SetRenderCustomDepth(true);
        Mesh->CustomDepthStencilValue = 1;
        FindReachableTiles();
    }
}

void APlayerUnit::DeselectUnit()
{
    if (bIsSelected)
    {
        bIsSelected = false;
        Mesh->SetRenderCustomDepth(false);
        ReachableTiles.Empty();
    }
}

void APlayerUnit::FindReachableTiles()
{
    ReachableTiles.Empty();

    FVector2D StartCoord(
        FMath::RoundToInt(GetActorLocation().X / 110.0f + 12.5f),
        FMath::RoundToInt(GetActorLocation().Y / 110.0f + 12.5f)
    );

    TArray<AActor*> TileActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), TileActors);

    // Build a Map of coordinates to tiles
    TMap<FVector2D, ATile*> TileMap;
    for (AActor* Actor : TileActors)
    {
        ATile* Tile = Cast<ATile>(Actor);
        if (Tile)
        {
            TileMap.Add(Tile->GetCoordinates(), Tile);
        }
    }

    // Now do BFS
    TQueue<TPair<FVector2D, int32>> Queue;
    TSet<FVector2D> Visited;

    Queue.Enqueue(TPair<FVector2D, int32>(StartCoord, 0));
    Visited.Add(StartCoord);

    const TArray<FVector2D> Directions = {
        FVector2D(1, 0),   // Right
        FVector2D(-1, 0),  // Left
        FVector2D(0, 1),   // Up
        FVector2D(0, -1)   // Down
    };

    while (!Queue.IsEmpty())
    {
        TPair<FVector2D, int32> Current;
        Queue.Dequeue(Current);

        FVector2D Coord = Current.Key;
        int32 Distance = Current.Value;

        if (Distance > 0)
        {
            ReachableTiles.Add(Coord);
        }

        if (Distance >= MovementRange)
            continue;

        for (const FVector2D& Dir : Directions)
        {
            FVector2D Neighbor = Coord + Dir;

            if (Visited.Contains(Neighbor))
                continue;

            ATile** TilePtr = TileMap.Find(Neighbor);
            if (TilePtr && *TilePtr)
            {
                ATile* Tile = *TilePtr;
                if (Tile->GetTileType() != ETileType::Empty)
                {
                    continue; // Obstacle blocks path
                }

                // Now check if another unit blocks this tile
                bool bBlockedByUnit = false;
                TArray<AActor*> UnitActors;
                UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerUnit::StaticClass(), UnitActors);

                for (AActor* UnitActor : UnitActors)
                {
                    APlayerUnit* OtherUnit = Cast<APlayerUnit>(UnitActor);
                    if (OtherUnit && OtherUnit != this)
                    {
                        FVector2D UnitCoord(
                            FMath::RoundToInt(OtherUnit->GetActorLocation().X / 110.0f + 12.5f),
                            FMath::RoundToInt(OtherUnit->GetActorLocation().Y / 110.0f + 12.5f)
                        );
                        if (UnitCoord == Neighbor)
                        {
                            bBlockedByUnit = true;
                            break;
                        }
                    }
                }

                if (!bBlockedByUnit)
                {
                    Visited.Add(Neighbor);
                    Queue.Enqueue(TPair<FVector2D, int32>(Neighbor, Distance + 1));
                }
            }
        }
    }
}
