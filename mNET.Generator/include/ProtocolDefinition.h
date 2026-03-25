#pragma once

#ifndef _H_PROTOCOL_DEFINITION_
#define _H_PROTOCOL_DEFINITION_

#include "MessageDefinition.h"
#include "IndentedStringStream.h"

#include <string>
#include <vector>

namespace mnet::definitions
{
    class ProtocolDefinition
    {
    public:
        static bool ReadFrom(const std::string& path, ProtocolDefinition& out);

        bool WriteHeaderFile(IndentedStringStream& stream);
        bool WriteSourceFile(IndentedStringStream& stream);

        const std::string GetProtocolHeaderGuardName() const;
        const std::string GetMessageReceiverHeaderGuardName() const;
        const std::string GetMessageReceiverClassName() const;
        const std::string GetNamespaceName() const { return GetName() + "Messages"; }
        const std::string GetProtocolFileName() const { return GetName() + "Protocol"; }
        const std::string GetReceiverFileName() const { return "Base" + GetName() + "Receiver"; }
        const std::string& GetName() const { return m_sName; }

        const std::vector<MessageDefinition>& Messages() const { return m_vMessages; }
    private:
        std::string m_sName;
        std::vector<MessageDefinition> m_vMessages;
        std::vector<std::string> m_vAdditionalIncludes;
    };
}


#endif // !_H_PROTOCOL_DEFINITION_




