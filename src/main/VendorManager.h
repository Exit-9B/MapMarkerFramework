#pragma once

class VendorManager final
{
public:
	~VendorManager() = default;
	VendorManager(const VendorManager&) = delete;
	VendorManager(VendorManager&&) = delete;
	VendorManager& operator=(const VendorManager&) = delete;
	VendorManager& operator=(VendorManager&&) = delete;

	static VendorManager* GetSingleton();

	void Load();

	RE::BGSListForm* GetVendorList(RE::BGSLocation* a_location);

private:
	VendorManager() = default;

	std::unordered_map<RE::BGSLocation*, RE::BGSListForm*> _vendorLists;
};
