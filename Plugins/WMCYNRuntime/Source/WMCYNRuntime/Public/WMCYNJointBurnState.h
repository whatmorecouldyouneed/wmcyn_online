#pragma once

#include "CoreMinimal.h"

/** Port of the Joint Roller CigaretteController owner logic. No actor or network dependency. */
struct FWMCYNJointBurnState
{
    static constexpr float BurnPerSecond = 0.005f;
    static constexpr float BoostMultiplier = 2.0f;
    static constexpr float MaxBoostParam = 0.8f;
    static constexpr float ExhaleSeconds = 1.5f;
    // Prefab ember travel in joint space, already multiplied by Unity's 100 cm per metre.
    static constexpr float FreshEmberHeight = 8.21f;
    static constexpr float FinishedEmberHeight = 3.46f;

    float Param = 0.0f;
    bool bBurning = false;
    bool bBoost = false;
    bool bExhaling = false;

    void Tick(const float DeltaSeconds)
    {
        if (bBurning)
        {
            if (MaxBoostParam < Param) bBoost = false;
            const float Delta = DeltaSeconds * BurnPerSecond * (bBoost ? BoostMultiplier : 1.0f);
            if (Param + Delta <= 1.0f)
            {
                Param += Delta;
            }
            else
            {
                Param = 1.0f;
                bBurning = false;
            }
        }
        if (ExhaleRemaining > 0.0f)
        {
            ExhaleRemaining -= DeltaSeconds;
            if (ExhaleRemaining <= 0.0f)
            {
                ExhaleRemaining = 0.0f;
                bExhaling = false;
            }
        }
    }

    void StartBurn()
    {
        bBurning = true;
        bBoost = false;
    }

    void StopBurn()
    {
        bBurning = false;
        bBoost = false;
    }

    void Reset()
    {
        bBurning = false;
        bBoost = false;
        Param = 0.0f;
    }

    void UseDown()
    {
        if (bBurning)
        {
            bBoost = true;
            bTriggerDownInBurning = true;
        }
    }

    void UseUp()
    {
        if (bTriggerDownInBurning)
        {
            bBoost = false;
            bExhaling = true;
            ExhaleRemaining = ExhaleSeconds;
        }
        bTriggerDownInBurning = false;
    }

    static float EmberHeight(const float InParam)
    {
        return FMath::Lerp(FreshEmberHeight, FinishedEmberHeight, InParam);
    }

private:
    bool bTriggerDownInBurning = false;
    float ExhaleRemaining = 0.0f;
};
