#include <iostream>
#include <vector>
#include <comdef.h>
#include <wbemidl.h>
#include <set>
#include <sstream>
#pragma comment(lib, "wbemuuid.lib")

using namespace std;

void get_CPU_Frequency()
{
	HRESULT hres;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		return;
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);

	if (FAILED(hres)) { 
		CoUninitialize(); 
		return; 
	}

	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) { 
		CoUninitialize(); 
		return; 
	}

	IWbemServices* pSvc = nullptr;
	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) { 
		pLoc->Release(); 
		CoUninitialize(); 
		return; 
	}

	HRESULT hr = CoSetProxyBlanket(pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE);

	if (FAILED(hr)) {
		cerr << "CoSetProxyBlanket failed: " << hex << hr << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return;
	}

	// Query WMI for MaxClockSpeed
	IEnumWbemClassObject* pEnumerator = nullptr;
	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT MaxClockSpeed FROM Win32_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);
	if (FAILED(hres)) { 
		pSvc->Release(); 
		pLoc->Release(); 
		CoUninitialize(); 
		return; 
	}

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;

	HRESULT hrClockSpeed = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
	if (SUCCEEDED(hrClockSpeed) && uReturn != 0) {
		VARIANT vt;
		VariantInit(&vt);

		pclsObj->Get(L"MaxClockSpeed", 0, &vt, 0, 0);
		double baseGHz = vt.intVal / 1000;
		cout << "Frequency: " << baseGHz << " GHz" << endl;

		VariantClear(&vt);
		pclsObj->Release();
	}

	pEnumerator->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();
}

void get_advanced_CPU_info() {
	// Type, family, model 
	int cpuInfo[4];
	__cpuid(cpuInfo, 1);
	int eax = cpuInfo[0];

	int stepping = eax & 0xF;
	int model = (eax >> 4) & 0xF;
	int family = (eax >> 8) & 0xF;
	int type = (eax >> 12) & 0x3;
	int extModel = (eax >> 16) & 0xF;
	int extFamily = (eax >> 20) & 0xFF;
	if (family == 6 || family == 15) {
		model += (extModel << 4);
	}
	if (family == 15) {
		family += extFamily;
	}

	cout << "Processor type: " << type << "\n";
	cout << "Family ID: " << family << "\n";
	cout << "Model ID: " << model << "\n";
	cout << "Stepping ID: " << stepping << "\n";
}

void get_CPU_info()
{
	// Vendor's signature
	int info[4]{};
	__cpuid(info, 0);
	char vendor[13]{};

	memcpy(vendor, &info[1], 4);
	memcpy(vendor + 4, &info[3], 4);
	memcpy(vendor + 8, &info[2], 4);

	cout << "Vendor signature: " << vendor << "\n";

	// Model
	char brand[49]{};
	int brandInfo[4];

	for (int i = 0; i < 3; i++) {
		__cpuid(brandInfo, 0x80000002 + i);
		memcpy(brand + i * 16, brandInfo, 16);
	}

	cout << "CPU: " << brand << "\n";

	// CPU Frequency
	get_CPU_Frequency();

	// number of cores and threads
	unsigned int eax, ebx, ecx, edx;
	int threadsPerCore = 0, threads = 0;

	for (int level = 0; level < 3; ++level) {
		int regs[4];
		__cpuidex(regs, 0xB, level);
		eax = regs[0];
		ebx = regs[1];
		ecx = regs[2];
		edx = regs[3];

		int levelType = (ecx >> 8) & 0xFF;
		if (levelType == 1) {
			threadsPerCore = ebx & 0xFFFF; // threads per core
		}
		if (levelType == 2) {
			threads = ebx & 0xFFFF; // number of threads
		}
	}

	int cores = threads / threadsPerCore; // number of cores

	cout << "Number of cores: " << cores << "\n";
	cout << "Number of threads: " << threads << "\n";
	cout << "Number of threads per core: " << threadsPerCore << "\n";

	int IA_extension_info[4];
	__cpuid(IA_extension_info, 1);
	vector<string> extensions;

	if (IA_extension_info[3] & (1 << 23)) extensions.push_back("MMX");
	if (IA_extension_info[3] & (1 << 25)) extensions.push_back("SSE");
	if (IA_extension_info[3] & (1 << 26)) extensions.push_back("SSE2");
	if (IA_extension_info[2] & (1 << 0))  extensions.push_back("SSE3");
	if (IA_extension_info[2] & (1 << 9))  extensions.push_back("SSSE3");
	if (IA_extension_info[2] & (1 << 12)) extensions.push_back("FMA3");
	if (IA_extension_info[2] & (1 << 19)) extensions.push_back("SSE4.1");
	if (IA_extension_info[2] & (1 << 20)) extensions.push_back("SSE4.2");
	if (IA_extension_info[2] & (1 << 25)) extensions.push_back("AES");
	if (IA_extension_info[2] & (1 << 28)) extensions.push_back("AVX");

	cout << "\nIA Extensions supported:\n";
	for (const auto& ext : extensions) {
		cout << "- " << ext << "\n";
	}

	cout << "\n";
	get_advanced_CPU_info();
}


void get_cache_info() {
	int info[4];
	int level = 0;

	while (true) {
		__cpuidex(info, 0x8000001D, level);  // extended cache enumration
		int cacheType = info[0] & 0x1F;      // EAX[4:0] - cache type

		if (cacheType == 0) { // no more caches
			break;
		}

		int cacheLevel = (info[0] >> 5) & 0x7;        // EAX[7:5] - cache level
		int selfInit = (info[0] >> 8) & 0x1;         // EAX[8] - self initializing
		int isFullyAssoc = (info[0] >> 9) & 0x1;     // EAX[9] - fully associative
		int maxThreads = ((info[0] >> 14) & 0xFFF) + 1; // EAX[25:14] - threads sharing this cache
		int maxCores = ((info[0] >> 26) & 0x3F) + 1;    // EAX[31:26] - cores per package

		int lineSize = (info[1] & 0xFFF) + 1;          // EBX[11:0] - line size in bytes
		int partitions = ((info[1] >> 12) & 0x3FF) + 1;  // EBX[21:12] - partitions
		int waysAssoc = ((info[1] >> 22) & 0x3FF) + 1;   // EBX[31:22] - ways of associativity
		int sets = info[2] + 1;                           // ECX - number of sets

		int cacheSizeBytes = waysAssoc * partitions * lineSize * sets;
		double cacheSizeKB = cacheSizeBytes / 1024.0;
		double cacheSizeMB = cacheSizeBytes / (1024.0 * 1024.0);

		bool complexIndexing = (info[3] >> 2) & 0x1;   // EDX[2] - complex indexing
		bool includesLower = (info[3] >> 1) & 0x1;     // EDX[1] - cache includes lower levels

		string cacheTypeStr;
		switch (cacheType) {
			case 1: cacheTypeStr = "Data Cache"; break;
			case 2: cacheTypeStr = "Instruction Cache"; break;
			case 3: cacheTypeStr = "Unified Cache"; break;
			default: cacheTypeStr = "Reserved / Unknown"; break;
		}

		cout << "Level: L" << cacheLevel << " (" << cacheTypeStr << ")\n";
		cout << "Size: " << cacheSizeKB << " KB (" << cacheSizeMB << " MB)\n";
		cout << "Line size: " << lineSize << " bytes\n";
		cout << "Partitions: " << partitions << "\n";
		cout << "Ways of associativity: " << waysAssoc << "\n";
		cout << "Sets: " << sets << "\n";
		cout << "Fully associative: " << (isFullyAssoc ? "Yes" : "No") << "\n";
		cout << "Self-initializing: " << (selfInit ? "Yes" : "No") << "\n";
		cout << "Max no. of threads sharing cache: " << maxThreads << "\n";
		cout << "Max no. of cores in package: " << maxCores << "\n";
		cout << "Complex indexing: " << (complexIndexing ? "Yes" : "No") << "\n";
		cout << "Includes lower levels: " << (includesLower ? "Yes" : "No") << "\n";

		cout << "\n\n";
		level++;
	}
}

void get_mem_paging_info()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	cout << "Page Size: " << sysInfo.dwPageSize / 1024.0 << " KB\n";
	cout << "Allocation Granularity: " << sysInfo.dwAllocationGranularity / 1024.0 << " KB\n";
	cout << "Minimum Application Virtual Address: 0x" << sysInfo.lpMinimumApplicationAddress << "\n";
	cout << "Maximum Application Virtual Address: 0x" << sysInfo.lpMaximumApplicationAddress << "\n";
}

string ws2s(const wstring& wstr) {
	if (wstr.empty()) return {};
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

void get_RAM_Info() {
	HRESULT hres;
	bool bCleanupCOM = false;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (SUCCEEDED(hres)) {
		bCleanupCOM = true; 
	}
	else if (hres == RPC_E_CHANGED_MODE) {
		bCleanupCOM = false;
	}
	else {
		return;
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
		if (bCleanupCOM) CoUninitialize();
		return;
	}

	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);

	if (FAILED(hres)) {
		if (bCleanupCOM) CoUninitialize();
		return;
	}

	IWbemServices* pSvc = nullptr;
	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

	if (FAILED(hres)) {
		pLoc->Release();
		if (bCleanupCOM) CoUninitialize();
		return;
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		if (bCleanupCOM) CoUninitialize();
		return;
	}

	IEnumWbemClassObject* pEnumerator = nullptr;
	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT Capacity, ConfiguredClockSpeed, Speed, Manufacturer, "
			"FormFactor, DeviceLocator, BankLabel, ConfiguredVoltage, MaxVoltage, "
			"SerialNumber, PartNumber, SMBIOSMemoryType, DataWidth, TotalWidth "
			"FROM Win32_PhysicalMemory"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		if (bCleanupCOM) CoUninitialize();
		return;
	}

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;
	int moduleIndex = 0;
	long long totalCapacityBytes = 0;
	std::set<wstring> channelSet;

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
		VARIANT vt;
		VariantInit(&vt);

		cout << "===== Module " << moduleIndex++ << " =====\n";

		// Manufacturer
		pclsObj->Get(L"Manufacturer", 0, &vt, 0, 0);
		cout << "Manufacturer: " << (vt.vt != VT_EMPTY ? ws2s(vt.bstrVal) : "Unknown") << "\n";
		VariantClear(&vt);

		// Capacity
		pclsObj->Get(L"Capacity", 0, &vt, 0, 0);
		__int64 capacityBytes = (vt.vt != VT_EMPTY ? _wtoi64(vt.bstrVal) : 0);
		__int64 capacityGB = capacityBytes / 1024 / 1024 / 1024;
		totalCapacityBytes += capacityBytes;
		cout << "Capacity: " << capacityGB << " GB\n";
		VariantClear(&vt);

		// Speed
		pclsObj->Get(L"ConfiguredClockSpeed", 0, &vt, 0, 0);
		unsigned int clk = (vt.vt != VT_EMPTY ? vt.uintVal : 0);
		cout << "Data Transfer Rate: " << clk << " MT/s\n";
		VariantClear(&vt);

		// Device Locator
		pclsObj->Get(L"DeviceLocator", 0, &vt, 0, 0);
		cout << "Device Locator: " << (vt.vt != VT_EMPTY ? ws2s(vt.bstrVal) : "Unknown") << "\n";
		VariantClear(&vt);

		// BankLabel
		pclsObj->Get(L"BankLabel", 0, &vt, 0, 0);
		wstring bankLabel = (vt.vt != VT_EMPTY ? vt.bstrVal : L"Unknown");
		cout << "Bank Label: " << ws2s(bankLabel) << "\n";
		VariantClear(&vt);

		// Channel detection logic
		size_t pos = bankLabel.find(L"CHANNEL ");
		if (pos != wstring::npos && bankLabel.size() > pos + 8) {
			wchar_t ch = bankLabel[pos + 8];
			channelSet.insert(wstring(1, ch));
		}

		// Form Factor
		pclsObj->Get(L"FormFactor", 0, &vt, 0, 0);
		string formFactorStr;
		switch (vt.uintVal) {
		case 8: formFactorStr = "DIMM"; break;
		case 12: formFactorStr = "SODIMM"; break;
		default: formFactorStr = "Other"; break;
		}
		cout << "Form Factor: " << formFactorStr << "\n";
		VariantClear(&vt);

		// Voltage
		pclsObj->Get(L"ConfiguredVoltage", 0, &vt, 0, 0);
		if (vt.vt != VT_EMPTY) {
			cout << "Configured Voltage: " << vt.uintVal / 1000.0 << " V\n";
		}
		VariantClear(&vt);

		// Serial & Part
		pclsObj->Get(L"SerialNumber", 0, &vt, 0, 0);
		cout << "Serial Number: " << (vt.vt != VT_EMPTY ? ws2s(vt.bstrVal) : "Unknown") << "\n";
		VariantClear(&vt);

		pclsObj->Get(L"PartNumber", 0, &vt, 0, 0);
		cout << "Part Number: " << (vt.vt != VT_EMPTY ? ws2s(vt.bstrVal) : "Unknown") << "\n";
		VariantClear(&vt);

		// Type
		pclsObj->Get(L"SMBIOSMemoryType", 0, &vt, 0, 0);
		unsigned int smType = (vt.vt != VT_EMPTY ? vt.uintVal : 0);
		string memTypeStr;
		switch (smType) {
		case 20: memTypeStr = "DDR"; break;
		case 21: memTypeStr = "DDR2"; break;
		case 22: memTypeStr = "DDR2 FB-DIMM"; break;
		case 24: memTypeStr = "DDR3"; break;
		case 26: memTypeStr = "DDR4"; break;
		case 30: memTypeStr = "DDR5"; break;
		case 34: memTypeStr = "DDR5"; break;
		default: memTypeStr = "Unknown"; break;
		}
		cout << "Type: " << memTypeStr << "\n";
		VariantClear(&vt);

		// ECC
		pclsObj->Get(L"DataWidth", 0, &vt, 0, 0);
		unsigned int dataWidth = (vt.vt != VT_EMPTY ? vt.uintVal : 0);
		VariantClear(&vt);

		pclsObj->Get(L"TotalWidth", 0, &vt, 0, 0);
		unsigned int totalWidth = (vt.vt != VT_EMPTY ? vt.uintVal : 0);
		VariantClear(&vt);

		cout << "Data width: " << dataWidth << " bits\n";
		cout << "Total width: " << totalWidth << " bits\n";
		cout << "ECC: " << ((totalWidth > dataWidth) ? "Yes" : "No") << "\n";

		pclsObj->Release();
		cout << "\n\n";
	}
	pEnumerator->Release();

	pEnumerator = nullptr;
	int totalSlots = 0;
	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT MemoryDevices FROM Win32_PhysicalMemoryArray"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hres) && pEnumerator) {
		if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
			VARIANT vt;
			VariantInit(&vt);
			pclsObj->Get(L"MemoryDevices", 0, &vt, 0, 0);
			totalSlots = (vt.vt != VT_EMPTY ? vt.uintVal : 0);
			pclsObj->Release();
			VariantClear(&vt);
		}
		pEnumerator->Release();
	}

	cout << "RAM Slots: Total = " << totalSlots
		<< ", Used = " << moduleIndex
		<< ", Free = " << (totalSlots - moduleIndex) << "\n";

	double totalGB = totalCapacityBytes / 1024.0 / 1024.0 / 1024.0;
	cout << "Total RAM Installed: " << totalGB << " GB\n";

	// Channel Configuration
	cout << "Channel Configuration: ";
	switch (channelSet.size()) {
	case 1: cout << "Single-channel\n"; break;
	case 2: cout << "Dual-channel\n"; break;
	case 3: cout << "Triple-channel\n"; break;
	case 4: cout << "Quad-channel\n"; break;
	default: cout << channelSet.size() << " channels\n"; break;
	}

	pSvc->Release();
	pLoc->Release();

	if (bCleanupCOM) {
		CoUninitialize();
	}
}



extern "C" __declspec(dllexport)
const char* getCPUInfo()
{
	static string result;
	ostringstream oss;
	streambuf* old = cout.rdbuf(oss.rdbuf());

	get_CPU_info();

	cout.rdbuf(old);
	result = oss.str();
	return result.c_str();
}


extern "C" __declspec(dllexport)
const char* getCacheInfo()
{
	static string result;
	ostringstream oss;
	streambuf* old = cout.rdbuf(oss.rdbuf());

	get_cache_info();

	cout.rdbuf(old);
	result = oss.str();
	return result.c_str();
}

extern "C" __declspec(dllexport)
const char* getMemPagingInfo()
{
	static string result;
	ostringstream oss;
	streambuf* old = cout.rdbuf(oss.rdbuf());

	get_mem_paging_info();

	cout.rdbuf(old);
	result = oss.str();
	return result.c_str();
}

extern "C" __declspec(dllexport)
const char* getRAMInfo()
{
	static string result;
	ostringstream oss;
	streambuf* old = cout.rdbuf(oss.rdbuf());

	get_RAM_Info();

	cout.rdbuf(old);
	result = oss.str();
	return result.c_str();
}