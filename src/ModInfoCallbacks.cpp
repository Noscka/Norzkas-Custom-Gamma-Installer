#include "../Headers/ModInfo.hpp"

void ModInfo::InitialResponseCallback(const std::wstring& statusString)
{
	UpdateLoadingScreen(statusString);
}

bool ModInfo::ProgressCallback(uint64_t len, uint64_t total)
{
	UpdateLoadingScreen((len * 100) / total);

	return true;
}