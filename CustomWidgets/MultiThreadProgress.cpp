#include "MultiThreadProgress.hpp"

ProgressStatus* MultiThreadProgress::RequestRegister()
{
	ThreadProgressBars.Append(new ProgressStatus(&ContentArea));
	ProgressStatus* newProgressBar = ThreadProgressBars[ThreadProgressBars.GetLastArrayIndex()];

	AddWidget(newProgressBar);

	return newProgressBar;
}

void MultiThreadProgress::RequestUnregister(ProgressStatus* progressBar)
{
	RemoveWidget(progressBar);

	ThreadProgressBars.ObjectRemove(progressBar);
}

ProgressStatus* MultiThreadProgress::RegisterProgressBar()
{
	ProgressStatus* returnProgressStatus = nullptr;
	QMetaObject::invokeMethod(this, "RequestRegister", Qt::BlockingQueuedConnection, Q_RETURN_ARG(ProgressStatus*, returnProgressStatus));

	if (returnProgressStatus == nullptr)
	{
		NosLib::Logging::CreateLog<wchar_t>(L"RegisterProgressBar returned a nullptr", NosLib::Logging::Severity::Error);
	}

	return returnProgressStatus;
}

void MultiThreadProgress::UnregisterProgressBar(ProgressStatus* progressBar)
{
	QMetaObject::invokeMethod(this, "RequestUnregister", Qt::BlockingQueuedConnection, Q_ARG(ProgressStatus*, progressBar));
}