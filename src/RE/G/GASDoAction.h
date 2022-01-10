#pragma once

#include "RE/G/GASActionBufferData.h"
#include "RE/G/GASExecuteTag.h"
#include "RE/G/GPtr.h"

namespace RE
{
	class GASDoAction : public GASExecuteTag
	{
	public:
		~GASDoAction() override;  // 00

		// override (GASExecuteTag)
		void Execute1(GFxSprite* a_sprite) override;                       // 01
		void Execute2(GFxSprite* a_sprite, std::int32_t a_arg2) override;  // 02
		bool Unk_04() override;                                            // 04 - { return 1; }

		// members
		GPtr<GASActionBufferData> data;  // 08
	};
	static_assert(sizeof(GASDoAction) == 0x10);
}
