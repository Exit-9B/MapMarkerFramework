#pragma once

class Settings
{
public:
	static auto GetSingleton() -> Settings*;

	void LoadSettings();

	struct
	{
		bool bObscuredUndiscovered;
		float fMarkerScale;
	} Map;

	struct
	{
		bool bObscuredUndiscovered;
	} HUD;

	struct
	{
		std::string sResourceFile;
	} Resources;
};
