#include "../Headers/ModProcessorThread.hpp"

#include "../Headers/InstallManager.hpp"
#include "../Headers/ModInfo.hpp"

ModProcessorThread::ModProcessorThread()
{
	InstallManager* instance = InstallManager::GetInstallManager();
	RegisteredStatusProgress = instance->ProgressContainer->RegisterProgressBar();

	connect(this, &ModProcessorThread::ModUpdateProgress, RegisteredStatusProgress, &ProgressStatus::UpdateProgress);
	connect(this, &ModProcessorThread::ModUpdateStatus, RegisteredStatusProgress, &ProgressStatus::UpdateStatus);
}

ModProcessorThread::~ModProcessorThread()
{
	InstallManager* instance = InstallManager::GetInstallManager();
	instance->ProgressContainer->UnregisterProgressBar(RegisteredStatusProgress);
}

void ModProcessorThread::ProcessMod()
{
	InstallManager* instance = InstallManager::GetInstallManager();

	if (ModCount == 0)
	{
		ModCount = ModInfo::ModInfoList.GetItemCount();
	}

	bool AllCompleted = false;

	/* Just to make the thread recheck all mods to make sure they are all installed */
	while (!AllCompleted)
	{
		AllCompleted = true;

		/* go through all mods in global static array */
		for (int i = 0; i <= ModInfo::ModInfoList.GetLastArrayIndex(); i++)
		{
			ModInfo* mod = ModInfo::ModInfoList[i];

			mod->WaitPriority(this);

			if (mod->GetModWorkState() != ModInfo::WorkState::NotStarted)
			{
				continue;
			}

			AllCompleted = false;
			mod->ProcessMod(this);

			CompleteCount++;
			instance->UpdateTotalProgress((CompleteCount * 100) / ModCount);
		}
	}
}