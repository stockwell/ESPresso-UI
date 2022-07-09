#include "Logging.hpp"

#include <chrono>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

Logging::Logging(bool autoFlush, const std::string& fileSuffix)
	: m_autoFlush(autoFlush)
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ssFileName;
	ssFileName << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S_") << fileSuffix;

	m_fileName = ssFileName.str();

	fs::create_directory("logs");
}

void Logging::FlushLog(bool newFile)
{
	if (! m_fileStream.is_open())
	{
		m_fileStream.open("logs/" + m_fileName + std::to_string(m_logCount) + ".csv");

		m_fileStream << "Seconds, Temperature, Pressure\n";
		m_fileStream.flush();
	}

	for (auto n = 0; n < m_data.size(); n++)
	{
		// write to file
		const auto& [temperature, pressure] = m_data[n];
		m_fileStream << float(m_dataPointsTotal + n) / 10.0f << ", " << temperature << ", " << pressure / 20 << '\n';
	}

	m_fileStream.flush();

	m_dataPointsTotal += m_data.size();

	if (newFile)
	{
		m_fileStream.close();
		++m_logCount;
		m_dataPointsTotal = 0;
	}

	m_data.clear();
}
