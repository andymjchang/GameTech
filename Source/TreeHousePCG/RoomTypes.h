#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomTypes.generated.h"

UENUM(BlueprintType)
enum class ERoomInputDirection : uint8
{
    North,
    South,
    West,
    East,
    Up,
    Down
};

/**
 * Directions for 8-way connectivity + Verticality
 */
UENUM(BlueprintType)
enum class ERoomDirection : uint8
{
    North       UMETA(DisplayName = "North (0, -1)"),
    NorthEast   UMETA(DisplayName = "North East (+1, -1)"),
    SouthEast   UMETA(DisplayName = "South East (+1, 0)"),
    South       UMETA(DisplayName = "South (0, +1)"),
    SouthWest   UMETA(DisplayName = "South West (-1, +1)"),
    NorthWest   UMETA(DisplayName = "North West (-1, 0)"),
    Up          UMETA(DisplayName = "Up (Z+1)"),
    Down        UMETA(DisplayName = "Down (Z-1)")
};

/**
 * The unique coordinate identifier for a room slot.
 * Axial Coordinates (Q, R) + Floor Index (Z).
 */
USTRUCT(BlueprintType)
struct FHexIndex
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
    int32 Q = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
    int32 R = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
    int32 FloorIndex = 0;

    FHexIndex() {}
    FHexIndex(int32 InQ, int32 InR, int32 InFloor) : Q(InQ), R(InR), FloorIndex(InFloor) {}

    // Equality operator required for TMap
    bool operator==(const FHexIndex& Other) const
    {
        return Q == Other.Q && R == Other.R && FloorIndex == Other.FloorIndex;
    }

    // Inequality operator
    bool operator!=(const FHexIndex& Other) const
    {
        return !(*this == Other);
    }

    FString ToString() const
    {
        return FString::Printf(TEXT("(Q:%d, R:%d, Z:%d)"), Q, R, FloorIndex);
    }
};

/**
 * Global hashing function required for TMap<FHexIndex, ...>
 * Must be outside the struct in the global scope.
 */
FORCEINLINE uint32 GetTypeHash(const FHexIndex& Index)
{
    // Combine hash of Q and R, then combine that result with FloorIndex
    return HashCombine(HashCombine(GetTypeHash(Index.Q), GetTypeHash(Index.R)), GetTypeHash(Index.FloorIndex));
}

/**
 * Represents the data of a single room node in the graph.
 */
USTRUCT(BlueprintType)
struct FRoomNode
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
    FHexIndex Location;

    // Bitmask: 1 = Open, 0 = Wall. Matches ERoomDirection order.
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Room")
    uint8 DoorMask = 0;

    // Asset defining visual/gameplay data (Enemies, Loot table, Theme)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    UDataAsset* RoomDefinition = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
    bool bIsActive = false;

    FRoomNode() {}
    FRoomNode(FHexIndex InLoc) : Location(InLoc), bIsActive(true) {}
};