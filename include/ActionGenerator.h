#pragma once

#include "SWFOutputStream.h"
#include <tsl/ordered_map.h>

class ActionGenerator
{
public:
	void Ready();
	auto GetCode() -> RE::GASActionBufferData*;

protected:
	struct Label
	{
		std::int16_t loc;
	};

	void FlushConstantPool();

	// Stack operations

	void Push(std::nullptr_t a_value);
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
	auto GetConstantPoolSize() -> std::int16_t;

	std::int16_t _pos = 0;
	SWFOutputStream _committed;
	SWFOutputStream _temporary;
	tsl::ordered_map<std::string, std::uint16_t> _constantPool;
};
