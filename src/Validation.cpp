#include <NCGI/validation.hpp>

bool Validation::ValidateStalkerAnomalyPath(const QString& q_path)
{
	std::filesystem::path path(q_path.toStdString());

	if (!std::filesystem::exists(path))
	{
		return false;
	}

	for (const std::string& subdirectory : stalker_sub_directories)
	{
		if (!std::filesystem::exists(path / subdirectory))
		{
			return false;
		}
	}

	return true;
}

NosLib::Result<uint64_t> Validation::GetFolderFreeSpace(const QString& folder)
{
	std::filesystem::path path(folder.toStdString());

	if (!std::filesystem::exists(path))
	{
		return {NosLib::GenericErrors::InvalidArgument, "file path doesn't exist"};
	}

	std::error_code ec;
	uint64_t free_space =  std::filesystem::space(path, ec).available;

	if (ec)
	{
		return { ec, std::format("getting free space failed because of: {}", ec.message()) };
	}

	return free_space;
}