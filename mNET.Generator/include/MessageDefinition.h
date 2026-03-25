#pragma once

#ifndef _H_MESSAGE_DEFINITION_
#define _H_MESSAGE_DEFINITION_

#include "FieldDefinition.h"

#include "IndentedStringStream.h"
#include "json.h"

#include <string>
#include <vector>

namespace mnet::definitions
{
    class MessageDefinition
    {
    public:
        static bool ReadFrom(const json::JObject& source, MessageDefinition& out);

        const std::vector<FieldDefinition>& Fields() const { return m_vFields; }
        const std::string& GetName() const { return m_sName; }
        size_t GetType() const { return m_lType; }

        const std::string GetClassName() const { return GetName() + "Message"; }
    private:
        size_t m_lType{ 0 };
        std::string m_sName;
        std::vector<FieldDefinition> m_vFields;
    };
}

#endif
