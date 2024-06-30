#include "../Headers/ModProcessorThread.hpp"

#include "../Headers/InstallManager.hpp"
#include "../Headers/ModInfo.hpp"

ModProcessorThread::ModProcessorThread()
{
	InstallManager* instance = InstallManager::GetInstallManager();
	RegisteredProgressBar = instance->ProgressContainer->RegisterProgressBar();

	connect(this, &ModProcessorThread::ModUpdateProgress, RegisteredProgressBar, &QProgressBar::setValue);
}

ModProcessorThread::~ModProcessorThread()
{
	InstallManager* instance = InstallManager::GetInstallManager();
	instance->ProgressContainer->UnregisterProgressBar(RegisteredProgressBar);
}

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

		mod->ProcessMod(this);

		CompleteCount++;
		instance->UpdateTotalProgress((CompleteCount * 100) / ModCount);
	}
}