#pragma once

#ifndef _H_FIELD_DEFINITION_
#define _H_FIELD_DEFINITION_

#include <string>

#include "json.h"

namespace mnet::definitions
{
    enum class FieldTypes {
        Unknown,
        Short,
        Bool,
        Int32,
        UInt64,
        Float,
        String,
    };

    FieldTypes FromString(const std::string& input);
	std::string ToNativeType(FieldTypes type);

    class FieldDefinition
    {
    public:
        static bool ReadFrom(const json::JObject& source, class FieldDefinition& out);

    public:
        const std::string& Name() const { return m_sName; }
        FieldTypes Type() const { return m_sType; }

    private:
        std::string m_sName;
        FieldTypes m_sType;
    };
}

#endif
