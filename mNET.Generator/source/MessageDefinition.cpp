#include "MessageDefinition.h"
#include "Utility.h"

#include <algorithm>
#include <string>
#include <fstream>

using namespace mnet::definitions;
using namespace mnet;

// JSON Fields
constexpr char const* NameField = "Name";
constexpr char const* TypeField = "Type";
constexpr char const* FieldsField = "Fields";

bool MessageDefinition::ReadFrom(const json::JObject& source, MessageDefinition& out)
{
	const json::JValue* element = *source[NameField];
	if (element) {
		out.m_sName = *element;
	}
	else {
		return false;
	}

	element = *source[TypeField];
	if (element) {
		out.m_lType = element->as_integer();
	}
	else {
		return false;
	}

	const json::JArray* array = *source[FieldsField];
	if (!array) {
		return false;
	}

	for (const auto& fieldJson : *array) {
		json::JObject* fieldObj = fieldJson->as_object();
		if (fieldObj == nullptr) {
			return false;
		}

		FieldDefinition field;
		if (!FieldDefinition::ReadFrom(*fieldObj, field)) {
			return false;
		}

		out.m_vFields.push_back(field);
	}

	return true;
}
