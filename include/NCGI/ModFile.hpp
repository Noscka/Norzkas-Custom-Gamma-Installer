#pragma once

#include <NosLib/HashTable.hpp>
#include <NosLib/Http/URL.hpp>
#include <NosLib/Http/HttpClient.hpp>
#include <NosLib/ErrorHandling.hpp>
#include <NosLib/Logging.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include "ModDB.hpp"

#include <string>
#include <functional>
#include <filesystem>
#include <atomic>

class ModInfo;

class ModFile
{
public:
	using Status = void(ModInfo::*)(const std::string&);
	using Progress = bool(ModInfo::*)(uint64_t, uint64_t);
protected:
	enum class HostType
	{
		ModDB,
		Github,
		GithubObjects,
		Unknown,
	};

	inline static bit7z::Bit7zLibrary lib = bit7z::Bit7zLibrary(L"7z.dll");
	inline static bit7z::BitFileExtractor extractor = bit7z::BitFileExtractor(lib);

	static NosLib::HashTable<std::string, ModFile*> fileHastTable;

	inline static std::filesystem::path download_directory_;
	inline static std::filesystem::path extract_directory_;

	NosLib::URL link_;
	std::filesystem::path file_name_;

	bool extracted_{ false };
	int usage_count_{ 1 };

	ModInfo* CallerPointer = nullptr;
	Status StatusCallback;
	Progress ProgressCallback;

	ModFile(const NosLib::URL& link, const std::string& file_name, const std::string& file_extension_overwrite = "");

public:
	ModFile() = default;

	static void set_directories(const std::filesystem::path& download_directory, const std::filesystem::path& extract_directory);
	static ModFile* register_file(const NosLib::URL& link, const std::string& file_name, const std::string& file_extension_overwrite = "");

	NosLib::Result<std::filesystem::path> get_file(ModInfo* callerPointer, const Status& statusCallback, const Progress& progressCallback);
	void release();
	std::string get_key();

protected:
	std::filesystem::path get_download_file_path();
	std::filesystem::path get_extract_file_path();

	std::string get_file_extension_from_header(const std::string& type);
	HostType determine_host_type(const std::string& host_name);
	NosLib::Result<void> download_file();
	NosLib::Result<void> get_and_save_file(httplib::Client* client, const NosLib::URL& url_path, const std::filesystem::path& path_offset);
	NosLib::Result<void> extract_file();
};