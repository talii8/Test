// Tile.cpp
#include "Tile.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "CommanderOfFate.h" // Needed to call Commander from Tile!

ATile::ATile()
{
    PrimaryActorTick.bCanEverTick = false;

    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (MeshAsset.Succeeded())
    {
        TileMesh->SetStaticMesh(MeshAsset.Object);
        TileMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.1f));
    }

    TileType = ETileType::Empty;
}

void ATile::BeginPlay()
{
    Super::BeginPlay();
    UpdateTileAppearance();

    // Bind Tile Click event
    OnClicked.AddDynamic(this, &ATile::OnTileClicked);
}

void ATile::SetCoordinates(int32 X, int32 Y)
{
    GridCoordinates = FVector2D(X, Y);
}

FVector2D ATile::GetCoordinates() const
{
    return GridCoordinates;
}

void ATile::SetTileType(ETileType Type)
{
    TileType = Type;
    UpdateTileAppearance();
}

ETileType ATile::GetTileType() const
{
    return TileType;
}

void ATile::UpdateTileAppearance()
{
    FString MaterialPath;
    switch (TileType)
    {
    case ETileType::Mountain:
        MaterialPath = TEXT("/Game/Obstacles/M_Mountain");
        break;
    case ETileType::Tree1:
        MaterialPath = TEXT("/Game/Obstacles/M_Tree1");
        break;
    case ETileType::Tree2:
        MaterialPath = TEXT("/Game/Obstacles/M_Tree2");
        break;
    default:
        return; // Empty stays with basic appearance
    }

    UMaterialInterface* Material = Cast<UMaterialInterface>(
        StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath)
    );

    if (Material)
    {
        TileMesh->SetMaterial(0, Material);
    }
}

void ATile::HighlightTile(bool bHighlight, FColor HighlightColor)
{
    if (bHighlight)
    {
        UMaterialInterface* HighlightMaterial = Cast<UMaterialInterface>(
            StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Game/Materials/M_TileHighlight.M_TileHighlight"))
        );

        if (HighlightMaterial)
        {
            TileMesh->SetMaterial(0, HighlightMaterial);
        }
    }
    else
    {
        UpdateTileAppearance(); // Restore the normal material
    }
}

void ATile::OnTileClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    UE_LOG(LogTemp, Warning, TEXT("Tile Clicked at (%d, %d)"), (int32)GridCoordinates.X, (int32)GridCoordinates.Y);

    ACommanderOfFate* Commander = Cast<ACommanderOfFate>(UGameplayStatics::GetActorOfClass(GetWorld(), ACommanderOfFate::StaticClass()));
    if (Commander)
    {
        Commander->HandleTileClicked(GridCoordinates);
    }
}
