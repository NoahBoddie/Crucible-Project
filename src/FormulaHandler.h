#pragma once



namespace Crucible
{
	class CombatData;

	enum class FormulaType : std::uint8_t
	{
		NoFormular,
		PoiseCalculation,
		PoiseRecoverCalculation,
		StaminaRecoverCalculation,
		MagickaRecoverCalculation,
	};

	//Static class used to handle various formulas that would be used in generic locations. Used to prevent the
	// use of lambdas or function pointers in various places.
	class FormulaHandler
	{
	public:
		static float UseFormular(FormulaType type, float value, RE::Actor* actor);
	};
}