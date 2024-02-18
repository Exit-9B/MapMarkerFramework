#pragma once

#include "SWFOutputStream.h"

namespace SWF
{
	class ActionGenerator
	{
	public:
		void Ready();
		auto GetCode() -> RE::GASActionBufferData*;

	protected:
		struct Label
		{
			bool defined = false;
			std::int16_t loc = 0;
		};

		// Stack operations

		void Push(std::nullptr_t a_value);
		void Push(std::int32_t a_value);
		void Push(float a_value);
		void Push(double a_value);
		void Push(const std::string& a_value, bool a_useConstantPool = true);

		// Arithmetic operators

		void Add();
		void Subtract();
		void Multiply();
		void Divide();

		// Numerical comparison

		void Equals2();

		// Logical operators

		void Not();

		// Control flow

		void Jump(Label& a_label);
		void If(Label& a_label);
		void L(Label& a_label);

		// Variables

		void GetVariable();
		void SetVariable();

		// ScriptObject actions

		void DefineLocal();
		void GetMember();
		void SetMember();

		// Other

		void InstanceOf();

	private:
		struct LabelRef
		{
			std::int16_t writePos;
			std::int16_t programCounter;
		};

		// Note: currently private because it could break labels
		void FlushConstantPool();

		auto GetPos() -> std::int16_t;

		void AddUndefinedLabel(Label& a_label, std::int16_t a_writePos, std::int16_t a_programPos);

		auto InitBuffer(std::size_t a_size) -> std::uint8_t*;

		RE::GASActionBufferData* _bufferData;
		SWFOutputStream _committed;
		SWFOutputStream _temporary;

		tsl::ordered_map<std::string, std::uint16_t> _constantPool;
		std::int16_t _constantPoolSize = 0;

		std::unordered_multimap<Label*, LabelRef> _undefinedLabels;
		std::unordered_map<std::int16_t, std::int16_t> _definedLabels;
	};
}
