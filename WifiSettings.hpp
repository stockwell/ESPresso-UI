#include <string>

class WifiSettings
{
public:
	explicit WifiSettings() = default;
	virtual ~WifiSettings() = default;

	struct ScanResult
	{
		std::string ssid;
		float RSSI;
	};

	virtual void startScan();



};
