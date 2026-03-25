#pragma once

#ifndef _H_PLOGGER_
#define _H_PLOGGER_

#include <mutex>
#include <thread>
#include <queue>
#include <filesystem>
#include <fstream>
#include <source_location>

#ifdef _PL_PLOGGER_IMPL_
#include <chrono>
#endif
#include <condition_variable>
#include <cstdint>
#include <format>
#include <sstream>
#include <string>

namespace planet::plogger {

	class PLoggerMessage {
	public:
		enum class MessageTypes {
			Unknown,
			Information,
			Warning,
			Error,
		};

	public:
		PLoggerMessage(MessageTypes type, const std::string& message, std::source_location location);

		void SetMethod(const std::string& method) { m_sMethod = method; }

		const std::string& GetMessage() const { return m_sMessage; }
		const MessageTypes GetType() const { return m_Type; }
		const std::string& GetMethod() const { return m_sMethod; }
		const std::string& GetFileName() const { return m_sFileName; }
	private:
		std::string m_sFileName;
		std::string m_sMethod;
		std::string m_sMessage;
		MessageTypes m_Type;
	};

	class PLogger
	{
	public:
		PLogger(std::filesystem::path logginPath, const std::string& applicationName);
		~PLogger();

		void Write(const std::string& message, const std::source_location location = std::source_location::current());
		void Warn(const std::string& message, const std::source_location location = std::source_location::current());
		void Error(const std::string& message, const std::source_location location = std::source_location::current());

		void SetMaxFileSize(size_t size) { m_maxFileSize = size; }

		void CloseLogger();
	private:

		std::filesystem::path GetNextAvailableOutputFileName();
		void ProcessingMethod();
		void SetupLoggingFile();
		void CheckLoggingFile();
		char MessageTypeToChar(PLoggerMessage::MessageTypes type);
		void WriteMessageToFile(PLoggerMessage& message);

		bool m_bRunning{ true };
		std::thread m_loggingThread;
		std::filesystem::path m_outputFilePath;
		std::fstream m_outputFile;
		std::mutex m_queueLock;
		std::queue<PLoggerMessage> m_messageQueue;
		std::condition_variable m_queueCv;
		uintmax_t m_maxFileSize;
	};

#ifdef _PL_PLOGGER_IMPL_
	PLogger::PLogger(std::filesystem::path logginPath, const std::string& applicationName)
	{
		m_outputFilePath = logginPath.append(applicationName + ".log");
		m_loggingThread = std::thread(&PLogger::ProcessingMethod, this);
		SetMaxFileSize(10000000);
	}

	PLogger::~PLogger()
	{
		CloseLogger();
	}

	void PLogger::Write(const std::string& message, std::source_location method)
	{
		std::scoped_lock lock(m_queueLock);
		m_messageQueue.emplace(PLoggerMessage::MessageTypes::Information, message, method);
		m_queueCv.notify_one();
	}

	void PLogger::Warn(const std::string& message, std::source_location method)
	{
		std::scoped_lock lock(m_queueLock);
		m_messageQueue.emplace(PLoggerMessage::MessageTypes::Warning, message, method);
		m_queueCv.notify_one();
	}

	void PLogger::Error(const std::string& message, std::source_location method)
	{
		std::scoped_lock lock(m_queueLock);
		m_messageQueue.emplace(PLoggerMessage::MessageTypes::Error, message, method);
		m_queueCv.notify_one();
	}

	void PLogger::CloseLogger()
	{
		if (!m_bRunning) {
			return;
		}

		m_bRunning = false;
		m_queueCv.notify_one();
		m_loggingThread.join();
		if (m_outputFile.is_open()) {
			m_outputFile.flush();
			m_outputFile.close();
		}
	}

	std::filesystem::path PLogger::GetNextAvailableOutputFileName()
	{
		std::string fileName = m_outputFilePath.filename().stem().string();

		int i = 0;
		while (true)
		{
			std::filesystem::path rootDir = m_outputFilePath.parent_path();
			auto& tempFileName = rootDir.append(fileName + "_" + std::to_string(i) + ".log");
			if (!std::filesystem::exists(tempFileName))
				return tempFileName;

			++i;
		}
	}

	void PLogger::ProcessingMethod()
	{
		std::unique_lock lk(m_queueLock);
		while (m_bRunning) {
			m_queueCv.wait(lk, [&] { return m_messageQueue.size() > 0 || !m_bRunning; });

			while (!m_messageQueue.empty()) {
				CheckLoggingFile();

				PLoggerMessage& msg = m_messageQueue.front();
				WriteMessageToFile(msg);
				m_messageQueue.pop();
			}

			m_outputFile.flush();
		}
	}

	void PLogger::SetupLoggingFile()
	{
		if (!std::filesystem::exists(m_outputFilePath.parent_path())) {
			std::filesystem::create_directory(m_outputFilePath.parent_path());
		}

		m_outputFile.open(m_outputFilePath, std::ios::app);
	}

	void PLogger::CheckLoggingFile()
	{
		if (!m_outputFile.is_open() || m_outputFile.bad())
		{
			m_outputFile.close();
			SetupLoggingFile();
		}

		std::uintmax_t fileSize = std::filesystem::file_size(m_outputFilePath);
		if (fileSize >= m_maxFileSize) {
			m_outputFile.close();
			std::filesystem::path newFilePath = GetNextAvailableOutputFileName();
			std::filesystem::rename(m_outputFilePath, newFilePath);
			SetupLoggingFile();
		}
	}

	char PLogger::MessageTypeToChar(PLoggerMessage::MessageTypes type)
	{
		switch (type) {
		case PLoggerMessage::MessageTypes::Information:
			return 'I';
		case PLoggerMessage::MessageTypes::Warning:
			return 'W';
		case PLoggerMessage::MessageTypes::Error:
			return 'E';
		default:
			return 'U';
		}
	}

	void PLogger::WriteMessageToFile(PLoggerMessage& message)
	{
		auto now = std::chrono::system_clock::now();
		std::string formattedTime = std::format("{:%Y-%m-%d %H:%M:%S}", now);
		std::stringstream builtMessage;
		builtMessage << "[" << formattedTime << "]";
		builtMessage << "[" << MessageTypeToChar(message.GetType()) << "]";
		builtMessage << "[" << message.GetFileName() << "]";
		builtMessage << "[" << message.GetMethod() << "]";

		builtMessage << message.GetMessage();
		m_outputFile << builtMessage.str() << "\n";
	}

	PLoggerMessage::PLoggerMessage(PLoggerMessage::MessageTypes type, const std::string& message, std::source_location location)
		: m_Type{ type }, m_sMessage{ message }, m_sFileName{ std::filesystem::path{location.file_name()}.filename().string() }, m_sMethod{ location.function_name() }
	{

	}

#endif
}

#endif // !_H_PLOGGER_


