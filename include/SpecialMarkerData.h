#pragma once

// We don't know much about this struct; there is a BSTArray of it in LocalMapMenu
struct SpecialMarkerData
{
	std::uint64_t unk00;      // 00
	RE::RefHandle refHandle;  // 08
	std::uint32_t unk0C;      // 0C
	std::uint64_t unk10;      // 10
	std::int32_t unk18;       // 18 - 1 = quest, 2 = player set
	std::int32_t unk1C;       // 1C - 1 = door
	std::int32_t unk20;       // 20
	std::uint32_t unk24;      // 24
	std::uint64_t unk28;      // 28
	std::uint64_t unk30;      // 38
};
static_assert(sizeof(SpecialMarkerData) == 0x38);
