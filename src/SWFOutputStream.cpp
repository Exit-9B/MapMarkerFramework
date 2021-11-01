#include "SWFOutputStream.h"
#include <algorithm>

void SWFOutputStream::Write(std::uint8_t a_byte)
{
	AlignByte();
	_os << static_cast<char>(a_byte);
	_pos++;
}

void SWFOutputStream::Write(std::string_view a_data)
{
	AlignByte();
	_os << a_data;
	_pos += a_data.length() + 1;
}

void SWFOutputStream::WriteUB(std::int32_t a_nBits, std::int64_t a_value)
{
	for (std::int32_t bit = 0; bit < a_nBits; bit++) {
		std::int8_t nb = static_cast<std::int8_t>((a_value >> (a_nBits - 1 - bit)) & 1);
		_tempByte += nb * (1 << (7 - _bitPos));
		_bitPos++;
		if (_bitPos == 8) {
			_bitPos = 0;
			Write(_tempByte);
			_tempByte = 0;
		}
	}
}

void SWFOutputStream::WriteSB(std::int32_t a_nBits, std::int64_t a_value)
{
	WriteUB(a_nBits, a_value);
}

void SWFOutputStream::WriteFB(std::int32_t a_nBits, float a_value)
{
	WriteSB(a_nBits, static_cast<std::int64_t>(a_value * (1 << 16)));
}

void SWFOutputStream::WriteUI8(std::uint8_t a_value)
{
	Write(a_value);
}

void SWFOutputStream::WriteUI16(std::uint16_t a_value)
{
	Write(a_value & 0xFF);
	Write(((a_value) >> 8) & 0xFF);
}

void SWFOutputStream::WriteUI32(std::uint32_t a_value)
{
	Write(a_value & 0xFF);
	Write((a_value >> 8) & 0xFF);
	Write((a_value >> 16) && 0xFF);
	Write((a_value >> 24) & 0xFF);
}

void SWFOutputStream::WriteFIXED(float a_value)
{
	WriteUI32(static_cast<std::uint32_t>(a_value * (1 << 16)));
}

void SWFOutputStream::WriteFIXED8(float a_value)
{
	WriteUI16(static_cast<std::uint16_t>(a_value * (1 << 8)));
}

void SWFOutputStream::WriteFLOAT(float a_value)
{
	union FloatConverter
	{
		float value;
		char bytes[sizeof(double)];
	};

	FloatConverter cv{ .value = a_value };

	Write(std::string_view(std::addressof(cv.bytes[0]), 4));
}

void SWFOutputStream::WriteDOUBLE(double a_value)
{
	union DoubleConverter
	{
		double value;
		char bytes[sizeof(double)];
	};

	DoubleConverter cv{ .value = a_value };

	Write(std::string_view(std::addressof(cv.bytes[4]), 4));
	Write(std::string_view(std::addressof(cv.bytes[0]), 4));
}

void SWFOutputStream::WriteMATRIX(const RE::GMatrix2D& a_value)
{
	float sx = a_value.data[0][0];
	float sy = a_value.data[1][1];
	float skx = a_value.data[0][1];
	float sky = a_value.data[1][0];
	float tx = a_value.data[0][2];
	float ty = a_value.data[1][2];

	bool hasScale = sx != 1.0f || sy != 1.0f;
	WriteUB(1, hasScale ? 1 : 0);
	if (hasScale) {
		std::int32_t nBits = GetNeededBitsF(sx);
		nBits = std::max(nBits, GetNeededBitsF(sy));

		WriteUB(5, nBits);
		WriteFB(nBits, sx);
		WriteFB(nBits, sy);
	}

	bool hasRotate = skx != 0.0f || sky != 0.0f;
	WriteUB(1, hasRotate ? 1 : 0);
	if (hasRotate) {
		std::int32_t nBits = GetNeededBitsF(skx);
		nBits = std::max(nBits, GetNeededBitsF(sky));

		WriteUB(5, nBits);
		WriteFB(nBits, skx);
		WriteFB(nBits, sky);
	}

	std::int32_t nTranslateBits = GetNeededBitsF(tx);
	nTranslateBits = std::max(nTranslateBits, GetNeededBitsF(ty));

	WriteUB(5, nTranslateBits);
	WriteFB(nTranslateBits, tx);
	WriteFB(nTranslateBits, ty);

	AlignByte();
}

void SWFOutputStream::WriteCXFORMWITHALPHA(const RE::GRenderer::Cxform& a_value)
{
	enum : std::size_t
	{
		kR,
		kG,
		kB,
		kA,
		kRGBA
	};

	enum
	{
		kMult,
		kAdd,
		kMultAdd
	};

	bool hasAddTerms =
		a_value.matrix[kR][kAdd] != 0.0f ||
		a_value.matrix[kG][kAdd] != 0.0f ||
		a_value.matrix[kB][kAdd] != 0.0f ||
		a_value.matrix[kA][kAdd] != 0.0f;

	WriteUB(1, hasAddTerms ? 1 : 0);

	bool hasMultTerms =
		a_value.matrix[kR][kMult] != 1.0f ||
		a_value.matrix[kG][kMult] != 1.0f ||
		a_value.matrix[kB][kMult] != 1.0f ||
		a_value.matrix[kA][kMult] != 1.0f;

	WriteUB(1, hasMultTerms ? 1 : 0);

	std::int32_t nBits = 1;
	if (hasMultTerms) {
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kR][kMult]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kG][kMult]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kB][kMult]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kA][kMult]));
	}
	if (hasAddTerms) {
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kR][kAdd]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kG][kAdd]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kB][kAdd]));
		nBits = std::max(nBits, GetNeededBitsF(a_value.matrix[kA][kAdd]));
	}

	WriteUB(4, nBits);
	if (hasMultTerms) {
		WriteFB(nBits, a_value.matrix[kR][kMult]);
		WriteFB(nBits, a_value.matrix[kG][kMult]);
		WriteFB(nBits, a_value.matrix[kB][kMult]);
		WriteFB(nBits, a_value.matrix[kA][kMult]);
	}
	if (hasAddTerms) {
		WriteFB(nBits, a_value.matrix[kR][kAdd]);
		WriteFB(nBits, a_value.matrix[kG][kAdd]);
		WriteFB(nBits, a_value.matrix[kB][kAdd]);
		WriteFB(nBits, a_value.matrix[kA][kAdd]);
	}

	AlignByte();
}

void SWFOutputStream::WriteSTRING(const char* a_value)
{
	if (!a_value) {
		return;
	}

	Write(std::string(a_value));
	Write(0);
}

void SWFOutputStream::WriteFILTERLIST(const RE::GArray<Filter>& a_value)
{
	assert(a_value.GetSize() <= 255);
	WriteUI8(static_cast<std::uint8_t>(a_value.GetSize()));
	for (auto& filter : a_value) {
		WriteFILTER(filter);
	}
}

void SWFOutputStream::WriteFILTER(const Filter& a_value)
{
	FilterType filterType = static_cast<FilterType>(a_value.filterType.underlying() & 0xF);

	switch (filterType) {
	case FilterType::kDropShadow:
		break;
	case FilterType::kBlur:
		break;
	case FilterType::kGlow:
		break;
	case FilterType::kBevel:
		break;
	case FilterType::kGradientGlow:
		break;
	case FilterType::kConvolution:
		break;
	case FilterType::kAdjustColor:
		break;
	case FilterType::kGradientBevel:
		break;
	}
}

auto SWFOutputStream::Get() -> std::string_view
{
	_os.flush();
	return _os.view();
}

void SWFOutputStream::Clear()
{
	_os.str(""s);
	_pos = 0;
	_bitPos = 0;
	_tempByte = 0;
}

void SWFOutputStream::AlignByte()
{
	if (_bitPos > 0) {
		_bitPos = 0;
		Write(_tempByte);
		_tempByte = 0;
	}
}

auto SWFOutputStream::GetNeededBitsS(std::int32_t a_value) -> std::int32_t
{
	std::int32_t counter = 32;
	std::uint32_t mask = 0x80000000;
	std::int32_t val = std::abs(a_value);
	while (((val & mask) == 0) && (counter > 0)) {
		mask >>= 1;
		counter -= 1;
	}
	return counter + 1;
}

auto SWFOutputStream::GetNeededBitsF(float a_value) -> std::int32_t
{
	return GetNeededBitsS(static_cast<std::int32_t>(a_value)) + 16;
}
