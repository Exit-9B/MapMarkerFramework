#pragma once

class Settings
{
public:
	static auto GetSingleton() -> Settings*;

	void LoadSettings();

	struct MapSettings
	{
		bool bObscuredUndiscovered;
		float fMarkerScale;
	};

	struct HUDSettings
	{
		bool bObscuredUndiscovered;
	};

	struct ResourcesSettings
	{
		std::string sResourceFile;
	};

	MapSettings Map;
	HUDSettings HUD;
	ResourcesSettings Resources;
};
