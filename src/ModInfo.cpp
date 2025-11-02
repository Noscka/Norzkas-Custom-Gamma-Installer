#include <NCGI/ModInfo.hpp>

#include <NCGI/InstallOptions.hpp>
#include <NCGI/InstallManager.hpp>
#include <NCGI/Validation.hpp>
#include <NCGI/ErrorCodes.hpp>

ModInfo::ModInfo(const std::string& main_name) :
	mod_prefix_index_(current_mod_prefix_index_++),
	main_name_(main_name),
	mod_type_(Type::Seperator)
{}

ModInfo::ModInfo(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& creator_name, const std::string& main_name, const std::string& original_link) :
	mod_prefix_index_(current_mod_prefix_index_++),
	from_paths_(from_paths_),
	creator_name_(creator_name),
	main_name_(main_name),
	original_link_(original_link),
	mod_file_handler_(ModFile::register_file(link, main_name)),
	mod_type_(Type::Standard)
{
}

ModInfo::ModInfo(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& to_path, const std::string& main_name, const bool& use_install_path, const std::string& custom_extension) :
	from_paths_(from_paths),
	to_path_(to_path_),
	main_name_(main_name),
	use_install_path_(use_install_path),
	mod_file_handler_(ModFile::register_file(link, main_name, custom_extension)),
	mod_type_(Type::Custom)
{
}

std::string ModInfo::generate_mod_name()
{
	switch (mod_type_)
	{
	case Type::Seperator:
		return std::format("{}- {}_separator", mod_prefix_index_, main_name_);

	case Type::Standard:
		return std::format("{}- {} {}", mod_prefix_index_, main_name_, creator_name_);

	case Type::Custom:
		return std::format("{}", main_name_);
	}

	return "Unknown Mod Type";
}

ModInfo* ModInfo::add_mod(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& to_path, const std::string& main_name, const bool& use_install_path, const std::string& custom_extension)
{
	ModInfo* mod = new ModInfo(link, from_paths, to_path, main_name, use_install_path, custom_extension);
	ModInfoList.push_back(mod);
	return mod;
}

NosLib::Result<void> ModInfo::parse_modpack_file(const std::filesystem::path& file_path)
{
	/* Add existance validation */

	std::ifstream file_stream(file_path, std::ios::binary);

	std::string line;
	while (std::getline(file_stream, line))
	{
		ModInfoList.push_back(parse_line(line));
	}
}

ModInfo* ModInfo::parse_line(std::string& line)
{
	std::vector<std::string> mod_info;

	NosLib::String::Split(&mod_info, line, '\t');

	for (int i = 0; i < mod_info.size(); i++)
	{
		std::string& entry = mod_info[i];
		entry = NosLib::String::Reduce(entry);
	}

	if (mod_info.size() == 1)
	{
		return new ModInfo(mod_info[0]);
	}

	std::vector<std::string> from_string_paths;
	NosLib::String::Split(&from_string_paths, mod_info[1], ':');

	bool hasRoot = false;

	for (std::string& entry : from_string_paths)
	{
		if (entry == "0" || entry == "\\")
		{
			entry = "\\";
			hasRoot = true;
		}
		else if (entry[0] != L'\\')
		{
			entry.insert(0, "\\");
		}

		if (entry.back() != L'\\')
		{
			entry.append("\\");
		}
	}

	if (!hasRoot)
	{
		from_string_paths.insert(from_string_paths.begin(), "\\");
	}

	std::vector<std::filesystem::path> from_paths;
	for (std::string& entry : from_string_paths)
	{
		from_paths.push_back(entry);
	}

	/* finally, if it has gotten here, it means the current line is a normal mod, pass in all the info to the constructor */
	return new ModInfo(mod_info[0], from_paths, mod_info[2], mod_info[3], mod_info[4]);
}

void ModInfo::process_mod()
{
	switch (mod_type_)
	{
	case Type::Standard:
		standard_mod_process();
		break;

	case Type::Custom:
		custom_mod_process();
		break;

	case Type::Seperator:
		separator_mod_process();
		break;

	default: /* TODO: CHANGE TO RESULT */
		//LogError("Undefined Mod Type tried to be processed", std::source_location::current());
		return;
	}

	if (mod_file_handler_ != nullptr)
	{
		mod_file_handler_->release();
		mod_file_handler_ = nullptr;
	}
}

void ModInfo::update_loading_screen(const std::string& status)
{
	//if (ProcessingThread != nullptr)
	//{
	//	ProcessingThread->UpdateModStatus(status);
	//	return;
	//}

	InstallManager* instance = InstallManager::GetInstallManager();
	//instance->UpdateModStatus(status);
}

void ModInfo::update_loading_screen(const int& precentage)
{
	InstallManager* instance = InstallManager::GetInstallManager();
	//instance->UpdateModProgress(precentage);
}

void ModInfo::update_loading_screen(const int& precentage, const std::string& status)
{
	update_loading_screen(precentage);
	update_loading_screen(status);
}

static NosLib::Result<void> copy_if_exists(const std::filesystem::path& from, const std::filesystem::path& to)
{
	if (!std::filesystem::exists(from))
	{
		return {NosLib::GenericErrors::InvalidArgument, std::format("{} doesn't exist", from.string())};
	}

	std::error_code ec;
	std::filesystem::create_directories(to, ec);
	if (ec)
	{
		return ec;
	}

	std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
	if (ec)
	{
		return ec;
	}

	return{};
}

NosLib::Result<void> ModInfo::standard_mod_process()
{
	update_loading_screen("Requesting File..."); /* TODO: ADD ACTUAL ERROR CHECKING */
	std::filesystem::path mod_file_path = mod_file_handler_->get_file(this, &ModInfo::InitialResponseCallback, &ModInfo::ProgressCallback).GetReturn();
	update_loading_screen("Received File");

	if (!std::filesystem::exists(mod_file_path))
	{
		return { NCGIError::ExtractionFailure, "Extracted mod file doesn't exist" };
	}

	update_loading_screen("Copying files...");

	for (std::filesystem::path& path : from_paths_)
	{
		std::filesystem::path from = (mod_file_path / path);
		std::filesystem::path to = (InstallOptions::GammaInstallPath / InstallInfo::ModDirectory / generate_mod_name());

		std::filesystem::create_directories(to);

		std::error_code ec;
		std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing, ec);
		if (ec)
		{
			NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, "Error while copying: {}", ec.message());
			continue;
		}

		for (std::filesystem::path& sub_dir : mod_sub_directories)
		{
			std::filesystem::path sub_from = from / sub_dir;
			std::filesystem::path sub_to = to / sub_dir;

			NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Copying \"{}\" to \"{}\"", sub_from.string(), sub_to.string());
			if (NosLib::Result<void> res = copy_if_exists(sub_from, sub_to); !res)
			{
				NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, res.GetAdditionalErrorMessage());
			}
			NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Copied \"{}\" to \"{}\"", sub_from.string(), sub_to.string());
		}
	}
	update_loading_screen("Finished Copying");
}

NosLib::Result<void> ModInfo::custom_mod_process()
{
	update_loading_screen("Requesting File..."); /* TODO: ADD ACTUAL ERROR CHECKING */
	std::filesystem::path mod_file_path = mod_file_handler_->get_file(this, &ModInfo::InitialResponseCallback, &ModInfo::ProgressCallback).GetReturn();
	update_loading_screen("Received File");

	if (!std::filesystem::exists(mod_file_path))
	{
		return { NCGIError::ExtractionFailure, "Extracted mod file doesn't exist" };
	}

	update_loading_screen("Copying files...");
	for (std::filesystem::path& path : from_paths_)
	{
		std::filesystem::path from = (mod_file_path / path);
		std::filesystem::path to = (use_install_path_ ? InstallOptions::GammaInstallPath / main_name_ : main_name_);

		std::filesystem::create_directories(to);
		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Copying \"{}\" to \"{}\"", from.string(), to.string());
		if (NosLib::Result<void> res = copy_if_exists(from, to); !res)
		{
			NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, res.GetAdditionalErrorMessage());
		}
		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Copied \"{}\" to \"{}\"", from.string(), to.string());
	}
	update_loading_screen("Finished Copying");
}

NosLib::Result<void> ModInfo::separator_mod_process()
{
	std::error_code ec;
	std::filesystem::create_directories(InstallOptions::GammaInstallPath / InstallInfo::ModDirectory / generate_mod_name(), ec);
	return ec;
}

void ModInfo::InitialResponseCallback(const std::string& statusString)
{
	update_loading_screen(statusString);
}

bool ModInfo::ProgressCallback(uint64_t len, uint64_t total)
{
	update_loading_screen((len * 100) / total);

	return true;
}