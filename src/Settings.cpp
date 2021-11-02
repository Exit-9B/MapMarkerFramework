#include "Settings.h"
#include "SimpleIni.h"

auto Settings::GetSingleton() -> Settings*
{
	static Settings singleton{};
	return std::addressof(singleton);
}

void Settings::LoadSettings()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(R"(.\Data\SKSE\Plugins\MapMarkerFramework.ini)");

	Map.bObscuredUndiscovered = ini.GetBoolValue("Map", "bObscuredUndiscovered", false);
	Map.fMarkerScale = static_cast<float>(ini.GetDoubleValue("Map", "fMarkerScale", 1.0));

	HUD.bObscuredUndiscovered = ini.GetBoolValue("HUD", "bObscuredUndiscovered", false);

	Resources.sResourceFile = ini.GetValue("Resources", "sResourceFile");
}
