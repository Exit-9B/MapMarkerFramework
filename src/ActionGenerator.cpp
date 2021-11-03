#include "ActionGenerator.h"

void ActionGenerator::Ready()
{
	FlushConstantPool();

	_committed.Write(0);
}

auto ActionGenerator::GetCode() -> RE::GASActionBufferData*
{
	// SkyrimSE 1.5.97.0: 0x017BC3F0
	static REL::Relocation<std::uintptr_t> GASActionBufferData_vtbl{ REL::ID(291566), 0x38 };

	auto bufferData = static_cast<RE::GASActionBufferData*>(
		RE::GMemory::Alloc(sizeof(RE::GASActionBufferData)));

	std::memset(bufferData, 0, sizeof(RE::GASActionBufferData));

	*reinterpret_cast<std::uintptr_t*>(bufferData) = GASActionBufferData_vtbl.get();

	auto code = _committed.Get();

	auto buffer = RE::GMemory::AllocAutoHeap(bufferData, code.size());
	memcpy(buffer, code.data(), code.size());
	bufferData->buffer = buffer;
	bufferData->size = code.size();

	bufferData->unk20 = 0;

	return bufferData;
}

void ActionGenerator::FlushConstantPool()
{
	if (!_constantPool.empty()) {

		std::uint16_t size = 0;
		std::uint16_t count = static_cast<std::uint16_t>(_constantPool.size());

		for (auto& [value, _] : _constantPool) {
			size += static_cast<std::uint16_t>(value.length() + 1);
		}

		_committed.WriteUI8(0x88);
		_committed.WriteUI16(2 + size);
		_committed.WriteUI16(count);

		for (auto& [value, _] : _constantPool) {
			_committed.WriteSTRING(value.data());
		}

		_constantPool.clear();
	}

	_committed.Write(_temporary.Get());
	_temporary.Clear();
}

void ActionGenerator::Push(const std::string& a_value, bool a_useConstantPool)
{
	if (a_useConstantPool) {
		std::uint16_t registerNum;

		auto item = _constantPool.find(a_value);
		if (item != _constantPool.end()) {
			registerNum = item->second;
		}
		else {
			registerNum = static_cast<std::uint16_t>(_constantPool.size());
			_constantPool[a_value] = registerNum;
		}

		if (registerNum < 0x100) {
			_temporary.WriteUI8(0x96);
			_temporary.WriteUI16(2);
			_temporary.WriteUI8(8);
			_temporary.WriteUI8(registerNum & 0xFF);
		}
		else {
			_temporary.WriteUI8(0x96);
			_temporary.WriteUI16(2);
			_temporary.WriteUI8(9);
			_temporary.WriteUI16(registerNum);
		}
	}
	else {
		_temporary.WriteUI8(0x96);
		_temporary.WriteUI16(2);
		_temporary.WriteUI8(0);
		_temporary.WriteSTRING(a_value.data());
	}
}

// Stack operations

void ActionGenerator::Push(float a_value)
{
	_temporary.WriteUI8(0x96);
	_temporary.WriteUI16(5);
	_temporary.WriteUI8(1);
	_temporary.WriteFLOAT(a_value);
}

void ActionGenerator::Push(double a_value)
{
	_temporary.WriteUI8(0x96);
	_temporary.WriteUI16(9);
	_temporary.WriteUI8(6);
	_temporary.WriteDOUBLE(a_value);
}

// Math actions

void ActionGenerator::Add()
{
	_temporary.WriteUI8(0x0A);
}

void ActionGenerator::Subtract()
{
	_temporary.WriteUI8(0x0B);
}

void ActionGenerator::Multiply()
{
	_temporary.WriteUI8(0x0C);
}

void ActionGenerator::Divide()
{
	_temporary.WriteUI8(0x0D);
}

// Variables

void ActionGenerator::GetVariable()
{
	_temporary.WriteUI8(0x1C);
}

void ActionGenerator::SetVariable()
{
	_temporary.WriteUI8(0x1D);
}

// Script Object actions

void ActionGenerator::DefineLocal()
{
	_temporary.WriteUI8(0x3C);
}

void ActionGenerator::GetMember()
{
	_temporary.WriteUI8(0x4E);
}

void ActionGenerator::SetMember()
{
	_temporary.WriteUI8(0x4F);
}
