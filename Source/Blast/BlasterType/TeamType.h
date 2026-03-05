#pragma once

UENUM()
enum class ETeam : uint8
{
	ET_Blue UMETA(DisplayName = "Blue Team"),
	ET_Red UMETA(DisplayName = "Red Team"),
	ET_NoTeam UMETA(DisplayName = "No Team")
};