#include "ProtocolDefinition.h"
#include "MessageDefinition.h"
#include "ProtocolWriters.h"
#include "Utility.h"

#include "json.h"
#include "JParser.h"

#include <fstream>
#include <string>

using namespace mnet::definitions;
using namespace mnet;

// JSON Fields
constexpr char const* NameField = "Name";
constexpr char const* AdditionalIncludesField = "AdditionalIncludes";
constexpr char const* MessagesField = "Messages";

bool ProtocolDefinition::ReadFrom(const std::string& path, ProtocolDefinition& out)
{
	std::ifstream f(path);
	json::JSON data = json::JParser::from_stream(f);
	f.close();

	if (!data.is_valid()) {
		return false;
	}

	json::JValue* nameValue = *data[NameField];
	if (nameValue) {
		out.m_sName = nameValue->as_string();
	}
	else {
		return false;
	}

	json::JArray* messagesArray = *data[MessagesField];
	if (!messagesArray) {
		return false;
	}

	for (auto& messageJson : *messagesArray)
	{
		auto messageObj = messageJson->as_object();
		if (!messageObj) {
			return false;
		}

		MessageDefinition def;
		if (!MessageDefinition::ReadFrom(*messageObj, def)) {
			return false;
		}

		out.m_vMessages.push_back(def);
	}

	json::JArray* includesArray = *data[AdditionalIncludesField];
	if (includesArray) {
		for (auto& include : *includesArray) {
			json::JValue* val = include->as_literal();
			if (!val || !val->is_string()) {
				return false;
			}

			out.m_vAdditionalIncludes.push_back(val->as_string());
		}
	}


	return true;
}

bool ProtocolDefinition::WriteHeaderFile(IndentedStringStream& ostream)
{
	std::string headerName = GetProtocolHeaderGuardName();
	std::string namespaceName = GetName() + "Messages";
	utility::ToUpper(headerName);

	definitions::writers::WriteDllApiMacros(ostream);

	ostream << "#ifndef " << headerName << "_\n";
	ostream << "#define " << headerName << "_\n";
	ostream << "\n";

	ostream << "#include \"mnet.h\"\n";
	ostream << "#include \"INetMessage.h\"\n";
	ostream << "#include <vector>\n";

	for (auto& include : m_vAdditionalIncludes) {
		if (include.ends_with(".h")) {
			ostream << "#include \"" << include << "\"";
		}
		else {
			ostream << "#include <" << include << ">\n";
		}
	}

	ostream << "\nnamespace " << namespaceName << "\n";
	ostream << "{\n";

	for (auto& message : m_vMessages) {
		if (!definitions::writers::GenerateMessageHeaderDefinition(message, ostream)) {
			return false;
		}

		ostream << "\n";
	}

	ostream << "}\n";
	ostream << "#endif // !" << headerName << "\n";
	return true;
}

bool ProtocolDefinition::WriteSourceFile(IndentedStringStream& stream)
{
	stream << "/*\t\t\tAUTO GENERATED PROTOCOL FILE\t\t\t*/\n";
	stream << "#include \"" << GetProtocolFileName() << ".hpp\"\n\n";
	std::string namespaceName = GetName() + "Messages";

	stream << "using namespace mnet;\n\n";
	stream << "using namespace " << namespaceName << ";\n\n";

	stream << "\n";
	for (auto& message : m_vMessages) {
		if (!definitions::writers::GenerateMessageSourceDefinition(message, stream)) {
			// TODO
			return false;
		}
		stream << "\n";
	}

	return true;
}

const std::string ProtocolDefinition::GetProtocolHeaderGuardName() const
{
	std::string s = "_H_" + GetName() + "_MESSAGES_";
	utility::ToUpper(s);
	return s;
}

const std::string ProtocolDefinition::GetMessageReceiverHeaderGuardName() const
{
	std::string s = "_H_" + GetName() + "_MESSAGE_RECEIVER_";
	utility::ToUpper(s);
	return s;
}

const std::string ProtocolDefinition::GetMessageReceiverClassName() const
{
	return "Base" + GetName() + "MessageReceiver";
}
