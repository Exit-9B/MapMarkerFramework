#pragma once

#include "SWFOutputStream.h"
#include <tsl/ordered_map.h>

class ActionGenerator
{
public:
	void Ready();
	auto GetCode() -> RE::GASActionBufferData*;

protected:
	void FlushConstantPool();

	// Stack operations

	void Push(const std::string& a_value, bool a_useConstantPool = true);
	void Push(double a_value);

	// Math actions

	void Add();
	void Subtract();
	void Multiply();
	void Divide();

	// Variables

	void GetVariable();
	void SetVariable();

	// ScriptObject actions

	void DefineLocal();
	void GetMember();
	void SetMember();

private:
	SWFOutputStream _committed;
	SWFOutputStream _temporary;
	tsl::ordered_map<std::string, std::uint16_t> _constantPool;
};
