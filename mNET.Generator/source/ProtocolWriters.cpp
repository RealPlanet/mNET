#include "ProtocolWriters.h"
#include "Utility.h"

bool mnet::definitions::writers::WriteDllApiMacros(IndentedStringStream& stream)
{
	stream << "/*\t\t\tAUTO GENERATED PROTOCOL FILE\t\t\t*/\n";
	stream << "/* General DLL API Macro to import/export these messages */\n";
	stream << "#ifndef MNET_PROTOCOL_FILE_DLL_API\n";
	stream << "\t#ifdef COMPILE_MNET_PROTOCOL_FILES\n";
	stream << "\t\t#define " << utility::DLL_API_NAME << " __declspec(dllexport)\n";
	stream << "\t#else\n";
	stream << "\t\t#define " << utility::DLL_API_NAME << " __declspec(dllimport)\n";
	stream << "\t#endif\n";
	stream << "#endif // !MNET_PROTOCOL_FILE_DLL_API\n";

	return true;
}

bool mnet::definitions::writers::GenerateMessageReceiverHeader(const ProtocolDefinition& def, IndentedStringStream& stream)
{
	std::string protocolHeaderFileName = def.GetProtocolFileName() + ".hpp";

	std::string headerName = def.GetMessageReceiverHeaderGuardName();
	std::string namespaceName = def.GetNamespaceName();

	if (!WriteDllApiMacros(stream)) {
		return false;
	}

	stream << "#ifndef " << headerName << "\n";
	stream << "#define " << headerName << "\n\n";

	stream << "#include \"mnet.h\"\n";
	stream << "#include \"IMessageReceiver.h\"\n";
	stream << "#include \"" << protocolHeaderFileName << "\"\n\n";

	stream << "namespace " << namespaceName << "\n";
	stream << "{\n";
	stream.indent();

	std::string className = def.GetMessageReceiverClassName();
	stream << "class " << className << "\n";
	stream << "\t: public mnet::IMessageReceiver\n";
	stream << "{\n";

	stream << "public:\n";
	stream.indent();
	stream << utility::DLL_API_NAME << " virtual void Process(mnet::Socket* sender, const mnet::INetMessage* message) override;\n";
	stream << utility::DLL_API_NAME << " virtual mnet::INetMessage* AllocateMessageOfType(size_t type) override;\n";
	stream << "mnet::Socket* GetCurrentSender() { return m_pCurrentSender; }\n";
	stream << "\n";

	for (auto& message : def.Messages()) {
		std::string name = message.GetClassName();
		stream << utility::DLL_API_NAME << " virtual void On" << name << "Received(const " << name << "* message) {}\n";
	}

	stream.dedent();

	stream << "\n";
	stream << "protected:\n";
	stream.indent();
	stream << "mnet::Socket* m_pCurrentSender{nullptr};";
	stream.dedent();

	stream << "\n};\n";

	stream.dedent();
	stream << "}\n\n";

	stream << "#endif // !" << headerName << "\n";

	return true;
}

bool mnet::definitions::writers::GenerateMessageReceiverSource(const ProtocolDefinition& def, IndentedStringStream& stream)
{
	std::string receiverHeaderFileName = def.GetReceiverFileName() + ".hpp";
	std::string protocolHeaderFileName = def.GetProtocolFileName() + ".hpp";

	std::string headerName = def.GetMessageReceiverHeaderGuardName();
	std::string namespaceName = def.GetNamespaceName();
	std::string className = def.GetMessageReceiverClassName();

	stream << "#include \"" << receiverHeaderFileName << "\"\n";
	stream << "#include \"" << protocolHeaderFileName << "\"\n";
	stream << "using namespace mnet;\n";
	stream << "using namespace " << namespaceName << ";\n\n";

	stream << "void " << className << "::Process(Socket* receiver, const INetMessage* message)\n";
	stream << "{\n";
	stream.indent();

	stream << "auto type = message->Type();\n";
	stream << "switch(type)\n";
	stream << "{\n";
	stream.indent();

	for (auto& msg : def.Messages()) {
		stream << "case " << msg.GetClassName() << "::TYPE:\n";
		stream << "{\n";
		stream.indent();
		stream << "On" << msg.GetClassName() << "Received((const " << msg.GetClassName() << "*)message);\n";
		stream << "break;\n";
		stream.dedent();
		stream << "}\n";
	}

	stream << "default:\n";
	stream << "{\n";
	stream.indent();
	stream << "OnUnknownMessageReceived(message);\n";
	stream << "break;\n";
	stream.dedent();
	stream << "}\n";
	stream.dedent();
	stream << "}\n";

	stream.dedent();
	stream << "}\n";
	stream << "\n";

	stream << "INetMessage* " << className << "::AllocateMessageOfType(size_t type)\n";
	stream << "{\n";
	stream.indent();

	stream << "switch(type)\n";
	stream << "{\n";
	stream.indent();

	for (auto& msg : def.Messages()) {
		stream << "case " << msg.GetClassName() << "::TYPE:\n";
		stream << "{\n";
		stream.indent();
		stream << "return new " << msg.GetClassName() << "();\n";
		stream.dedent();
		stream << "}\n";
	}

	stream << "default:\n";
	stream << "{\n";
	stream.indent();
	stream << "return nullptr;\n";
	stream.dedent();
	stream << "}\n";

	stream.dedent();
	stream << "}\n";
	stream.dedent();
	stream << "}\n";
	stream << "\n";

	return true;
}

bool mnet::definitions::writers::GenerateMessagePrefixComment(const MessageDefinition& def, IndentedStringStream& stream, bool indentStream)
{
	std::string className = def.GetClassName();

	if (indentStream) {
		stream.indent();
	}

	stream << "////////////////////////////////////\n";
	stream << "//\t\t MESSAGE:: " << className << "\n";
	stream << "//\t\t NUM OF FIELDS:: " << def.Fields().size() << "\n";
	stream << "////////////////////////////////////\n";

	if (indentStream) {
		stream.dedent();
	}

	stream << "\n";
	return true;
}

bool mnet::definitions::writers::GenerateMessageHeaderDefinition(const MessageDefinition& def, IndentedStringStream& stream)
{
	std::string className = def.GetClassName();

	if (!GenerateMessagePrefixComment(def, stream, true)) {
		return false;
	}

	stream.indent();
	stream << "class " << className << "\n";
	stream << "\t: public mnet::INetMessage\n";
	stream << "{\n";
	stream << "public:\n";

	stream.indent();
	stream << "static constexpr size_t TYPE = " << def.GetType() << ";\n";
	stream << utility::DLL_API_NAME << " " << className << "();\n";
	stream << "\n";
	stream.dedent();

	stream << "public:\n";

	stream.indent();
	stream << utility::DLL_API_NAME << " virtual bool Decode(const std::vector<char>& data) override;\n";
	stream << utility::DLL_API_NAME << " virtual std::vector<char> Encode() override;\n";
	stream << "\n";

	for (auto& field : def.Fields()) {
		std::string fieldType = "const " + mnet::definitions::ToNativeType(field.Type()) + "&";

		stream << utility::DLL_API_NAME << " " << fieldType << " Get" << field.Name() << "() const { return m_" << field.Name() << "; }\n";
		stream << utility::DLL_API_NAME << " " << "void Set" << field.Name() << "(" << fieldType << " value) { m_" << field.Name() << " = value; }\n";
	}
	stream.dedent();

	stream << "private:\n";
	stream.indent();
	for (auto& field : def.Fields()) {
		stream << mnet::definitions::ToNativeType(field.Type()) << " m_" << field.Name() << "{};\n";
	}
	stream.dedent();

	stream << "};\n";
	stream.dedent();
	return true;
}

bool mnet::definitions::writers::GenerateMessageSourceDefinition(const MessageDefinition& def, IndentedStringStream& stream)
{
	std::string className = def.GetClassName();

	if (!GenerateMessagePrefixComment(def, stream, false)) {
		return false;
	}

	stream << className << "::" << className << "()\n";
	stream.indent();
	stream << ": INetMessage(TYPE)\n";
	stream.dedent();
	stream << "{\n";
	stream << "}\n\n";

	stream << "bool " << className << "::Decode(const std::vector<char>& data)\n";
	stream << "{\n";
	stream.indent();

	if (def.Fields().size() > 0) {
		stream << "size_t offset{ 0 };\n";
		stream << "// BEGIN DECODING OF FIELDS IN ORDER OF DEFINITION\n";
	}
	else {
		stream << "// THIS MESSAGE DOES NOT HAVE ANY FIELDS\n";
	}

	for (auto& field : def.Fields()) {
		stream << "// Decode field: " << field.Name() << "\n";
		if (field.Type() == definitions::FieldTypes::String) {
			std::string fieldSizeMemberName = "string" + field.Name() + "Length";
			stream << "size_t " << fieldSizeMemberName << "{0};\n";


			stream << "std::memcpy(&" << fieldSizeMemberName << ", &data[0] + offset, sizeof(size_t));\n";
			stream << "offset += " << "sizeof(size_t);\n";
			stream << "m_" << field.Name() << ".resize(" << fieldSizeMemberName << ");\n";
			stream << "std::memcpy(m_" << field.Name() << ".data(), &data[0] + offset, " << fieldSizeMemberName << " );\n";
			stream << "offset += " << fieldSizeMemberName << ";\n";
			stream << "\n";
			continue;
		}

		stream << "std::memcpy(&m_" << field.Name() << ", &data[0] + offset, " << "sizeof(m_" << field.Name() << ")" << " );\n";
		stream << "offset += " << "sizeof(m_" << field.Name() << ")" << ";\n";
		stream << "\n";
	}

	stream << "return true;\n";
	stream.dedent();
	stream << "}\n\n";

	stream << "std::vector<char> " << className << "::Encode()\n";
	stream << "{\n";
	stream.indent();
	stream << "std::vector<char> buffer = CreateEncodingBuffer();\n";

	if (def.Fields().size() > 0) {
		stream << "// Start offset based on size of buffer which typically holds space for type and length of message\n";
		stream << "size_t offset { buffer.size() };\n";
		stream << "\n";
		stream << "// BEGIN ENCODING OF FIELDS IN ORDER OF DEFINITION\n";
	}
	else {
		stream << "// THIS MESSAGE DOES NOT HAVE ANY FIELDS\n";
	}

	// TODO :: If the message has no dynamic fields we could preallocate the buffer size to avoid multiple resizes.

	for (auto& field : def.Fields()) {
		stream << "// Encode field: " << field.Name() << "\n";
		if (field.Type() == definitions::FieldTypes::String) {
			std::string tempVarName = field.Name() + "_size";
			stream << "size_t " << tempVarName << " = m_" << field.Name() << ".size(); \n";

			// Allocate space for string length and string data
			stream << "buffer.resize(buffer.size() + sizeof(size_t) + " << tempVarName << ");\n";
			stream << "std::memcpy(&buffer[0] + offset, &" << tempVarName << ", sizeof(size_t));\n";
			stream << "offset += sizeof(size_t);\n";
			stream << "std::memcpy(&buffer[0] + offset, m_" << field.Name() << ".data(), " << field.Name() << "_size);\n";
			stream << "offset += " << field.Name() << "_size;\n";
			stream << "\n";
			continue;
		}

		// Write immutable field
		stream << "buffer.resize(buffer.size() + sizeof(m_" << field.Name() << "));\n";
		stream << "std::memcpy(&buffer[0] + offset, &m_" << field.Name() << ", sizeof(m_" << field.Name() << "));\n";
		stream << "offset += sizeof(m_" << field.Name() << ");\n";
		stream << "\n";
	}

	stream << "EncodeBaseMessageData(buffer); \n";
	stream << "return buffer;\n";
	stream.dedent();
	stream << "}\n\n";

	return true;
}
