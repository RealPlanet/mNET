#define _PL_ARGPARSER_IMPL_
#include "ArgParser.h"
#include "ConsoleWriter.h"

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>

#include "Utility.h"
#define _PL_PLOGGER_IMPL_
#include "PLogger.h"
#include "ProtocolDefinition.h"
#include "ProtocolWriters.h"

namespace mnet
{
    static std::vector<std::string> s_ProtocolDefinitionFiles;
    static std::string s_OutputFolder;

    static void AddToProtocolFiles(const std::string& s) {
        s_ProtocolDefinitionFiles.push_back(s);
    }

    static void SetOutputFolder(const std::string& s) {
        s_OutputFolder = s;
    }

    static bool GenerateCPPHeaderFile(definitions::ProtocolDefinition& protocol) {

        // Step 1 :: Define protocol .hpp file
        mnet::IndentedStringStream protocolStream{};
        if (!protocol.WriteHeaderFile(protocolStream)) {
            planet::writer::ConsoleWrite("Could not generate protocol C++ header file from protocol definition", planet::writer::BG_RED);
            return false;
        }

        std::filesystem::path outputPath = s_OutputFolder;
        outputPath.append(protocol.GetProtocolFileName() + ".hpp");
        if (!utility::WriteStreamToFile(outputPath, protocolStream)) {
            planet::writer::ConsoleWrite("Could not write protocol C++ header file at " + outputPath.string(), planet::writer::BG_RED);
            return false;
        }

        // Step 2 :: Define message receiver .hpp file
        mnet::IndentedStringStream receiverStream{};
        if (!definitions::writers::GenerateMessageReceiverHeader(protocol, receiverStream)) {
            planet::writer::ConsoleWrite("Could not generate receiver C++ header file from protocol definition", planet::writer::BG_RED);
            return false;
        }

        std::filesystem::path receiverOutputPath = s_OutputFolder;
        receiverOutputPath.append(protocol.GetReceiverFileName() + ".hpp");
        if (!utility::WriteStreamToFile(receiverOutputPath, receiverStream)) {
            planet::writer::ConsoleWrite("Could not write receiver C++ header file at: " + receiverOutputPath.string(), planet::writer::BG_RED);
            return false;
        }

        return true;
    }

    static bool GenerateCPPSourceFile(definitions::ProtocolDefinition& protocol) {

        mnet::IndentedStringStream protocolStream{};
        if (!protocol.WriteSourceFile(protocolStream)) {
            planet::writer::ConsoleWrite("Could not generate protocol C++ header file from protocol definition...", planet::writer::BG_RED);
            return false;
        }

        std::filesystem::path outputPath = s_OutputFolder;
        outputPath.append(protocol.GetProtocolFileName() + ".cpp");
        if (!utility::WriteStreamToFile(outputPath, protocolStream)) {
            planet::writer::ConsoleWrite("Could not write protocol C++ source file at " + outputPath.string(), planet::writer::BG_RED);
            return false;
        }
        // Step 2 :: Define message receiver .hpp file
        mnet::IndentedStringStream receiverStream{};
        if (!definitions::writers::GenerateMessageReceiverSource(protocol, receiverStream)) {
            planet::writer::ConsoleWrite("Could not generate receiver C++ source file from protocol definition...", planet::writer::BG_RED);
            return false;
        }

        std::filesystem::path receiverOutputPath = s_OutputFolder;
        receiverOutputPath.append(protocol.GetReceiverFileName() + ".cpp");
        if (!utility::WriteStreamToFile(receiverOutputPath, receiverStream)) {
            planet::writer::ConsoleWrite("Could not write receiver C++ source file at: " + receiverOutputPath.string(), planet::writer::BG_RED);
            return false;
        }

        return true;
    }

    static bool GenerateProtocolFile(const std::string& source) {

        definitions::ProtocolDefinition protocolDefinition;
        if (!definitions::ProtocolDefinition::ReadFrom(source, protocolDefinition)) {
            planet::writer::ConsoleWrite("Could not parse file for protocol definition...", planet::writer::BG_RED);
            return false;
        }

        if (!GenerateCPPHeaderFile(protocolDefinition)) {
            planet::writer::ConsoleWrite("C++ Header generation failed", planet::writer::BG_RED);
            return false;
        }

        if (!GenerateCPPSourceFile(protocolDefinition)) {
            planet::writer::ConsoleWrite("C++ Source generation failed", planet::writer::BG_RED);
            return false;
        }

        return true;
    }
}

namespace mnet::generator
{
    planet::plogger::PLogger logger = planet::plogger::PLogger("./logs/", "mNET_Generator");
}

static bool GenerateFiles() {
    if (mnet::s_ProtocolDefinitionFiles.size() == 0) {
        planet::writer::ConsoleWrite("\tNo valid protocol files have been provided!", planet::writer::Code::FG_YELLOW);
        return false;
    }

    if (mnet::s_OutputFolder == "") {
        planet::writer::ConsoleWrite("\tOutput folder not provided!", planet::writer::Code::BG_RED);
        return false;
    }

    if (!std::filesystem::exists(mnet::s_OutputFolder) ||
        !std::filesystem::is_directory(mnet::s_OutputFolder)) {
        if (!std::filesystem::create_directory(mnet::s_OutputFolder)) {
            planet::writer::ConsoleWrite("\tCould not create the output folder for generated files!", planet::writer::Code::BG_RED);
            return false;
        }
    }

    auto oldFiles = mnet::s_ProtocolDefinitionFiles;
    mnet::s_ProtocolDefinitionFiles.clear();
    for (auto& file : oldFiles)
    {
        if (std::filesystem::exists(file)) {
            mnet::s_ProtocolDefinitionFiles.push_back(file);
            continue;
        }

        planet::writer::ConsoleWrite("\tFile '" + file + "' does not exist, ignoring it!", planet::writer::Code::FG_YELLOW);
    }

    bool anyError = false;
    for (auto& file : mnet::s_ProtocolDefinitionFiles) {
        planet::writer::ConsoleWrite("## Generating protocol files for: " + file + " ##", planet::writer::Code::FG_GREEN);

        if (mnet::GenerateProtocolFile(file))
        {
            planet::writer::ConsoleWrite("##\tGeneration succesful!\t##", planet::writer::Code::FG_GREEN);
        }
        else
        {
            planet::writer::ConsoleWrite("##\tGeneration failed!\t##", planet::writer::Code::BG_RED);
            anyError = true;
        }
    }

    planet::writer::ConsoleWrite("## mNET Protocol Generator Wizard has finished ##", planet::writer::Code::FG_GREEN);
    return !anyError;
}

static bool AttemptGenerationFromAllArguments(int argc, char* argv[]) {
    mnet::s_OutputFolder = "";
    mnet::s_ProtocolDefinitionFiles.clear();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.starts_with("\"")) {
            arg = arg.substr(1);
        }

        if (arg.ends_with("\"")) {
            arg = arg.substr(0, arg.size() - 1);
        }
        if (std::filesystem::exists(arg) &&
            std::filesystem::is_regular_file(arg)) {
            mnet::s_ProtocolDefinitionFiles.push_back(arg);
        }
        else {
            planet::writer::ConsoleWrite("\t\"" + arg + "\" is not a valid protocol file path!", planet::writer::Code::FG_YELLOW);
        }
    }

    if (mnet::s_ProtocolDefinitionFiles.size() > 0) {
        std::filesystem::path path = std::filesystem::canonical( mnet::s_ProtocolDefinitionFiles[0]);
        mnet::s_OutputFolder = path.parent_path().string();
        planet::writer::ConsoleWrite("Will output files at: " + mnet::s_OutputFolder, planet::writer::Code::FG_YELLOW);
    }

    return GenerateFiles();
}

int main(int argc, char* argv[])
{
    mnet::generator::logger.Write("The application is starting...");

    planet::writer::ConsoleWrite("## mNET Protocol Generator Wizard ##", planet::writer::Code::FG_GREEN);

    planet::argparser::ArgParser argParser([](const std::string& err) {
        planet::writer::ConsoleWrite(err + '\n', planet::writer::Code::BG_RED);
        mnet::generator::logger.Error(err);
        });

    argParser.RegisterArgument("f|file=", mnet::AddToProtocolFiles);
    argParser.RegisterArgument("o|output=", mnet::SetOutputFolder);
    if (!argParser.Parse(argc, argv)) {
        planet::writer::ConsoleWrite("Could not parse program arguments! Attempting to generate from drag-n-drop...", planet::writer::Code::BG_RED);
        mnet::generator::logger.Warn("Attempting drag-n-drop execution!");

        if (AttemptGenerationFromAllArguments(argc, argv))
        {
            mnet::generator::logger.Write("Processed drag-n-drop correctly!");
            return 0;
        }

        mnet::generator::logger.Error("drag-n-drop did not complete succesfully!");
        planet::writer::ConsoleWrite("Press [Enter] to close the console...", planet::writer::BG_BLUE);
        char c = getchar();
        return -1;
    }

    int result = GenerateFiles() ? 0 : -1;
    mnet::generator::logger.CloseLogger();
    return result;
}
