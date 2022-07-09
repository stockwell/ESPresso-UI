#pragma once

#include <string>
#include <vector>
#include <fstream>

class Logging
{
public:
	Logging(bool autoFlush, const std::string& fileSuffix);
	~Logging() = default;

	using DataPoint = std::pair<float, float>;

	void AddData(const DataPoint& data)
	{
		m_data.push_back(data);

		if (m_autoFlush)
			FlushLog(false);
	}

	void FlushLog(bool newFile = true);

private:
	bool m_autoFlush	= false;
	size_t m_logCount	= 1;
	size_t m_dataPointsTotal = 0;

	std::vector<DataPoint> m_data;
	std::string m_fileName;

	std::ofstream m_fileStream;
};
