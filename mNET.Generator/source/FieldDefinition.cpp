#include "FieldDefinition.h"

#include "JSON.h"

#include <algorithm>
#include <string>
#include <fstream>

using namespace mnet::definitions;
using namespace mnet;

constexpr char const* NameField = "Name";
constexpr char const* TypeField = "Type";

bool FieldDefinition::ReadFrom(const json::JObject& source, FieldDefinition& out)
{
	const json::JValue* element = *(source[NameField]);
	if (element && element->is_string()) {
		out.m_sName = *element;
	}
	else {
		return false;
	}

	element = *source[TypeField];
	if (element && element->is_string()) {
		out.m_sType = mnet::definitions::FromString(*element);
	}
	else {
		return false;
	}

	return true;
}

FieldTypes mnet::definitions::FromString(const std::string& input)
{
	static std::map<std::string, FieldTypes> typeMap = {
		{"short", FieldTypes::Short},
		{"int32", FieldTypes::Int32},
		{"uint64", FieldTypes::UInt64},
		{"float", FieldTypes::Float},
		{"string", FieldTypes::String},
		{"bool", FieldTypes::Bool},
	};

	auto it = typeMap.find(input);
	if (it == typeMap.end()) {
		return FieldTypes::Unknown;
	}

	return it->second;
}

std::string mnet::definitions::ToNativeType(FieldTypes type)
{
	switch (type) {
	case FieldTypes::Short:
		return "short";
	case FieldTypes::Bool:
		return "bool";
	case FieldTypes::Int32:
		return "int32_t";
	case FieldTypes::UInt64:
		return "size_t";
	case FieldTypes::Float:
		return "float";
	case FieldTypes::String:
		return "std::string";
	default:
		return "";
	}
}
