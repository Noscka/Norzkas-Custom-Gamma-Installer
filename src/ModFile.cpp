#include <NCGI/ModFile.hpp>
#include <NCGI/ModInfo.hpp>
#include <NCGI/Github.hpp>
#include <NCGI/ErrorCodes.hpp>

#include <NosLib/String.hpp>

#include <fstream>
#include <filesystem>
#include <format>

NosLib::HashTable<std::string, ModFile*> ModFile::fileHastTable(&ModFile::get_key, 400);

ModFile::ModFile(const NosLib::URL& link, const std::string& file_name, const std::string& file_extension_overwrite) :
	link_(link),
	file_name_(file_name)
{
	if (!file_extension_overwrite.empty())
	{
		file_name_.replace_extension(file_extension_overwrite);
	}
}

void ModFile::set_directories(const std::filesystem::path& download_directory, const std::filesystem::path& extract_directory)
{
	download_directory_ = download_directory;
	NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Download Directory set to {}", download_directory.string());

	extract_directory_ = extract_directory;
	NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "Extract Directory set to {}", extract_directory.string());
}

ModFile* ModFile::register_file(const NosLib::URL& link, const std::string& file_name, const std::string& file_extension_overwrite)
{
	if (ModFile** searched_file = fileHastTable.Find(link.GetFullURL()); searched_file != nullptr)
	{
		ModFile* f = *searched_file;
		f->usage_count_++;
		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "File {} for {} found in register, {} uses", f->file_name_.string(), f->link_.GetFullURL(), f->usage_count_);
		return f;
	}

	ModFile* f = new ModFile(link, file_name, file_extension_overwrite);
	fileHastTable.Insert(f);
	NosLib::Logging::CreateLog(NosLib::Logging::Severity::Debug, "File {} for {} not found in register", f->file_name_.string(), f->link_.GetFullURL());
	return f;
}

NosLib::Result<std::filesystem::path> ModFile::get_file(ModInfo* callerPointer, const Status& statusCallback, const Progress& progressCallback)
{
	/* Update Callbacks */
	CallerPointer = callerPointer;
	StatusCallback = statusCallback;
	ProgressCallback = progressCallback;

	if (extracted_)
	{
		return get_extract_file_path();
	}

	if (NosLib::Result<void> res = download_file(); !res)
	{
		return { res.ErrorCode(), res.GetAdditionalErrorMessage() };
	}

	if (NosLib::Result<void> res = extract_file(); !res)
	{
		return { res.ErrorCode(), res.GetAdditionalErrorMessage() };
	}

	extracted_ = true;

	return get_extract_file_path();
}

void ModFile::release()
{
	usage_count_--;

	if (usage_count_ > 0)
	{
		return;
	}

	std::error_code ec;
	std::filesystem::remove_all(get_download_file_path(), ec);
	if (ec)
	{
		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, "Failed removing \"{}\" because of {}", get_download_file_path().string(), ec.message());
	}

	std::filesystem::remove_all(get_extract_file_path(), ec);
	if (ec)
	{
		NosLib::Logging::CreateLog(NosLib::Logging::Severity::Error, "Failed removing \"{}\" because of {}", get_extract_file_path().string(), ec.message());
	}

	fileHastTable.Remove(get_key());
	delete this;
}

std::string ModFile::get_key()
{
	return link_.GetFullURL();
}

std::filesystem::path ModFile::get_download_file_path()
{
	return download_directory_ / file_name_;
}

std::filesystem::path ModFile::get_extract_file_path()
{
	return extract_directory_ / file_name_;
}

std::string ModFile::get_file_extension_from_header(const std::string& type)
{
	if (type.find("application/zip") != std::string::npos)
	{
		return ".zip";
	}

	if (type.find("application/x-7z-compressed") != std::string::npos)
	{
		return ".7z";
	}

	if (type.find("application/x-rar-compressed") != std::string::npos || type.find("application/vnd.rar") != std::string::npos)
	{
		return ".rar";
	}

	return ".ERROR";
}

ModFile::HostType ModFile::determine_host_type(const std::string& host_name)
{
	if (host_name.find("moddb") != std::string::npos)
	{
		return HostType::ModDB;
	}

	if (host_name.find("objects.githubusercontent") != std::string::npos)
	{
		return HostType::GithubObjects;
	}

	if (host_name.find("github") != std::string::npos)
	{
		return HostType::Github;
	}

	return HostType::Unknown;
}

NosLib::Result<void> ModFile::download_file()
{
	std::filesystem::create_directories(download_directory_);

	NosLib::HttpClient::Ptr download_client = nullptr;
	NosLib::URL download_link = link_;

	switch (determine_host_type(link_.GetDomain()))
	{
	case HostType::ModDB:
	{
		download_client = ModDB::CreateDownloadClient();
		NosLib::Result<NosLib::URL> res = ModDB::get_download_url(link_);
		if (!res)
		{
			return { res.ErrorCode(), res.GetAdditionalErrorMessage() };
		}
		download_link = *res;
		break;
	}

	case HostType::GithubObjects:
		download_client = Github::CreateDownloadObjectsClient();
		break;

	case HostType::Github:
		download_client = Github::CreateDownloadClient();
		break;

	default:
		return { NCGIError::UnknownProvider, std::format("unknown mod provider for {}", link_.GetFullURL()) };
	}

	if (download_client == nullptr)
	{
		return { NosLib::GenericErrors::NullPointer, std::format("Download client for {} was nullptr", link_.GetFullURL()) };
	}

	return get_and_save_file(download_client.get(), download_link, download_directory_);
}

NosLib::Result<void> ModFile::get_and_save_file(httplib::Client* client, const NosLib::URL& url_path, const std::filesystem::path& path_offset)
{
	std::ofstream download_stream;

	auto response_callback = [&](const httplib::Response& response)
	{
		if (file_name_.extension().empty())
		{
			file_name_.replace_extension(get_file_extension_from_header(response.headers.find("Content-Type")->second));
		}

		std::string statusText = std::format("Downloading \"{}\"", file_name_.string());
		(CallerPointer->*StatusCallback)(statusText);

		download_stream.open(path_offset / file_name_, std::ios::binary | std::ios::trunc);
		return true;
	};

	auto data_callback = [&](const char* data, size_t data_length)
	{
		download_stream.write(data, data_length);
		return true;
	};

	auto progress_callback = [&](uint64_t len, uint64_t total)
	{
		return (CallerPointer->*ProgressCallback)(len, total);
	};

	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
	httplib::Result res = client->Get(url_path.GetSubdir(), response_callback, data_callback, progress_callback);

	if (!res)
	{
		return { NCGIError::HttpError, httplib::to_string(res.error()) };
	}

	if (res->status != httplib::OK_200)
	{
		return { NCGIError::HttpError, std::format("unexpected status {} | reason: {}", res->status, res->reason) };
	}

	download_stream.close();
	return {};
}

NosLib::Result<void> ModFile::extract_file()
{
	std::filesystem::create_directories(get_extract_file_path());
	NosLib::Logging::CreateLog(NosLib::Logging::Severity::Info, "Extracting \"{}\" to \"{}\"", get_download_file_path().string(), get_extract_file_path().string());

	uint64_t total_file_size = 1;
	extractor.setTotalCallback([&total_file_size](uint64_t total_size)
	{
		total_file_size = total_size;
	});

	extractor.setProgressCallback([&](uint64_t processed_size)
	{
		return (CallerPointer->*ProgressCallback)(processed_size, total_file_size);
	});

	try
	{
		(CallerPointer->*StatusCallback)(std::format("Extracting \"{}\"", file_name_.string()));
		extractor.extract(get_download_file_path().wstring(), get_extract_file_path().wstring());
	}
	catch (const bit7z::BitException& ex)
	{
		std::string error_message = std::format("The following files failed to extract because {}", ex.what());
		for (std::pair<std::wstring, std::error_code> entry : ex.failedFiles())
		{
			error_message += std::format(" | {} : {}", NosLib::String::ToString(entry.first), entry.second.message());
		}
		return { NCGIError::ExtractionFailure , error_message };
	}

	NosLib::Logging::CreateLog(NosLib::Logging::Severity::Info, "Extracted \"{}\" to \"{}\"", get_download_file_path().string(), get_extract_file_path().string());
	return {};
}