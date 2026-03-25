#pragma once

#ifndef _H_MESSAGE_RECEIVER_WRITER_
#define _H_MESSAGE_RECEIVER_WRITER_

#include "ProtocolDefinition.h"
#include "IndentedStringStream.h"

namespace mnet::definitions::writers {
    bool WriteDllApiMacros(IndentedStringStream& stream);

    bool GenerateMessageReceiverHeader(const ProtocolDefinition& def, IndentedStringStream& stream);
    bool GenerateMessageReceiverSource(const ProtocolDefinition& def, IndentedStringStream& stream);

    bool GenerateMessagePrefixComment(const MessageDefinition& def, IndentedStringStream& stream, bool indentStream);
    bool GenerateMessageHeaderDefinition(const MessageDefinition& def, IndentedStringStream& stream);
    bool GenerateMessageSourceDefinition(const MessageDefinition& def, IndentedStringStream& stream);
}

#endif // !_H_MESSAGE_RECEIVER_WRITER_


