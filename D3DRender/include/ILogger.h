#pragma once

#include <string>

namespace d3drender
{
	enum class Severity
	{
		Error,
		Warn,
		Info
	};

	class ILogger
	{
	public:
		virtual ~ILogger() = default;

		virtual void Log(Severity severity, const std::string& record, const char* fileName, int line) = 0;
	};

	using ILoggerPtr = std::shared_ptr<ILogger>;
}