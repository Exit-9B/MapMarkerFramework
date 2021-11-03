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

	auto iniPath = fmt::format(R"(.\Data\SKSE\Plugins\{}.ini)"sv, Version::PROJECT);
	ini.LoadFile(iniPath.data());

	Map.bObscuredUndiscovered = ini.GetBoolValue("Map", "bObscuredUndiscovered", false);
	Map.fMarkerScale = static_cast<float>(ini.GetDoubleValue("Map", "fMarkerScale", 1.0));

	HUD.bObscuredUndiscovered = ini.GetBoolValue("HUD", "bObscuredUndiscovered", false);

	Resources.sResourceFile = ini.GetValue("Resources", "sResourceFile");
}
