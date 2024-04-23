#include "SWF/ActionGenerator.h"

namespace SWF
{
	void ActionGenerator::Ready()
	{
		_undefinedLabels.clear();

		std::int16_t constantPoolSize = _constantPoolSize;
		FlushConstantPool();

		_committed.Write(0);

		auto view = _committed.Get();
		auto code = InitBuffer(view.size());
		std::memcpy(code, view.data(), view.size());

		for (auto& [writePos, offset] : _definedLabels) {
			std::int16_t pos = writePos + constantPoolSize;
			code[pos] = offset & 0xFF;
			code[pos + 1] = (offset >> 8) & 0xFF;
		}
	}

	auto ActionGenerator::GetCode() -> RE::GASActionBufferData*
	{
		return _bufferData;
	}

	void ActionGenerator::FlushConstantPool()
	{
		if (!_constantPool.empty()) {

			std::uint16_t size = _constantPoolSize - 3;
			std::uint16_t count = static_cast<std::uint16_t>(_constantPool.size());

			_committed.WriteUI8(0x88);
			_committed.WriteUI16(size);
			_committed.WriteUI16(count);

			for (auto& [value, _] : _constantPool) {
				_committed.WriteSTRING(value.data());
			}

			_constantPool.clear();
			_constantPoolSize = 0;
		}

		_committed.Write(_temporary.Get());
		_temporary.Clear();
	}

	// Stack operations

	void ActionGenerator::Push([[maybe_unused]] std::nullptr_t a_value)
	{
		_temporary.WriteUI8(0x96);
		_temporary.WriteUI16(1);
		_temporary.WriteUI8(2);
	}

	void ActionGenerator::Push(std::int32_t a_value)
	{
		_temporary.WriteUI8(0x96);
		_temporary.WriteUI16(5);
		_temporary.WriteUI8(7);
		_temporary.WriteSI32(a_value);
	}

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

	void ActionGenerator::Push(const std::string& a_value, bool a_useConstantPool)
	{
		if (a_useConstantPool) {
			std::uint16_t registerNum;

			auto item = _constantPool.find(a_value);
			if (item != _constantPool.end()) {
				registerNum = item->second;
			}
			else {
				if (_constantPool.empty()) {
					_constantPoolSize += 5;
				}

				registerNum = static_cast<std::uint16_t>(_constantPool.size());
				_constantPool[a_value] = registerNum;
				_constantPoolSize += static_cast<std::uint16_t>(a_value.length() + 1);
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

	// Arithmetic operators

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

	// Numerical comparison

	void ActionGenerator::Equals2()
	{
		_temporary.WriteUI8(0x49);
	}

	// Logical operators

	void ActionGenerator::Not()
	{
		_temporary.WriteUI8(0x12);
	}

	// Control flow

	void ActionGenerator::Jump(Label& a_label)
	{
		std::int16_t programCounter = GetPos() + 5;

		_temporary.WriteUI8(0x99);
		_temporary.WriteUI16(2);

		if (!a_label.defined) {
			std::int16_t writePos = GetPos();
			AddUndefinedLabel(a_label, writePos, programCounter);
		}

		_temporary.WriteSI16(a_label.loc - programCounter);
	}

	void ActionGenerator::If(Label& a_label)
	{
		std::int16_t programCounter = GetPos() + 5;

		_temporary.WriteUI8(0x9D);
		_temporary.WriteUI16(2);

		if (!a_label.defined) {
			std::int16_t writePos = GetPos();
			AddUndefinedLabel(a_label, writePos, programCounter);
		}

		_temporary.WriteSI16(a_label.loc - programCounter);
	}

	void ActionGenerator::L([[maybe_unused]] Label& a_label)
	{
		a_label.defined = true;
		a_label.loc = GetPos();

		auto [begin, end] = _undefinedLabels.equal_range(std::addressof(a_label));
		for (auto it = begin; it != end; ++it) {
			auto& labelRef = it->second;
			std::int16_t offset = a_label.loc - labelRef.programCounter;
			_definedLabels[labelRef.writePos] = offset;
		}
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

	// Other

	void ActionGenerator::InstanceOf()
	{
		_temporary.WriteUI8(0x54);
	}

	auto ActionGenerator::GetPos() -> std::int16_t
	{
		std::int16_t committedPos = static_cast<std::int16_t>(_committed.GetPos());
		std::int16_t tempPos = static_cast<std::int16_t>(_temporary.GetPos());

		return committedPos + tempPos;
	}

	void ActionGenerator::AddUndefinedLabel(
		Label& a_label,
		std::int16_t a_writePos,
		std::int16_t a_programPos)
	{
		_undefinedLabels.insert(
			{
				std::addressof(a_label),
				LabelRef{
					a_writePos,
					a_programPos,
				}
			});
	}

	auto ActionGenerator::InitBuffer(std::size_t a_size) -> std::uint8_t*
	{
		static REL::Relocation<std::uintptr_t> GASActionBufferData_vtbl{
			Offset::GASActionBufferData::Vtbl
		};

		_bufferData = static_cast<RE::GASActionBufferData*>(
			RE::GMemory::Alloc(sizeof(RE::GASActionBufferData)));

		std::memset(_bufferData, 0, sizeof(RE::GASActionBufferData));

		*reinterpret_cast<std::uintptr_t*>(_bufferData) = GASActionBufferData_vtbl.get();

		auto buffer = RE::GMemory::AllocAutoHeap(_bufferData, a_size);

		_bufferData->buffer = buffer;
		_bufferData->size = a_size;
		_bufferData->unk20 = 0;

		return static_cast<std::uint8_t*>(buffer);
	}
}
