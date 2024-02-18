#pragma once

namespace SWF
{
	class SWFOutputStream
	{
	public:
		using Filter = RE::GFxPlaceObjectUnpackedData::Filter;
		using FilterType = RE::GFxPlaceObjectUnpackedData::FilterType;

		void Write(std::uint8_t a_value);
		void Write(std::string_view a_data);

		void WriteUB(std::int32_t a_nBits, std::int64_t a_value);
		void WriteSB(std::int32_t a_nBits, std::int64_t a_value);
		void WriteFB(std::int32_t a_nBits, float a_value);
		void WriteSI16(std::int16_t a_value);
		void WriteSI32(std::int32_t a_value);
		void WriteUI8(std::uint8_t a_value);
		void WriteUI16(std::uint16_t a_value);
		void WriteUI32(std::uint32_t a_value);
		void WriteLong(std::uint64_t a_value);
		void WriteFIXED(double a_value);
		void WriteFIXED8(float a_value);
		void WriteFLOAT(float a_value);
		void WriteDOUBLE(double a_value);
		void WriteSTRING(const char* a_value);
		void WriteRGBA(RE::GColor a_value);
		void WriteMATRIX(const RE::GMatrix2D& a_value);
		void WriteCXFORMWITHALPHA(const RE::GRenderer::Cxform& a_value);
		void WriteFILTERLIST(const RE::GArray<Filter>& a_value);
		void WriteFILTER(const Filter& a_value);

		auto Get() -> std::string_view;
		auto GetPos() -> std::int64_t;
		void Clear();

	private:
		void AlignByte();

		static auto GetNeededBitsS(std::int32_t a_value) -> std::int32_t;
		static auto GetNeededBitsF(float a_value) -> std::int32_t;

		std::ostringstream _os;
		std::int64_t _pos = 0;
		std::uint8_t _bitPos = 0;
		std::uint8_t _tempByte = 0;
	};
}
