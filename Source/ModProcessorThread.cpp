#include "../Headers/ModProcessorThread.hpp"

#include "../Headers/InstallManager.hpp"
#include "../Headers/ModInfo.hpp"


void ModProcessorThread::ProcessMod()
{
	InstallManager* instance = InstallManager::GetInstallManager();

	if (ModCount == 0)
	{
		ModCount = ModInfo::modInfoList.GetItemCount();
	}

	/* go through all mods in global static array */
	for (ModInfo* mod : ModInfo::modInfoList)
	{
		if (mod->GetModWorkState() != ModInfo::WorkState::NotStarted)
		{
			continue;
		}

		mod->ProcessMod();

		CompleteCount++;
		instance->UpdateTotalProgress((CompleteCount * 100) / ModCount);
	}
}