#pragma once

#include <NosLib/String.hpp>
#include <NosLib/Http/URL.hpp>
#include <NosLib/Http/HttpClient.hpp>
#include <NosLib/ErrorHandling.hpp>

#include <format>
#include <source_location>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <NCGI/ModFile.hpp>

/* Sub directories that are inside each mod folder */
inline std::vector<std::filesystem::path> mod_sub_directories({ "fomod", "appdata", "bin", "db", "gamedata", "tools" });

class ModInfo
{
protected:
	enum class Type
	{
		Seperator,
		Standard,
		Custom,
	};

	static inline int current_mod_prefix_index_{ 0 };

	/* Mod Params */
	Type mod_type_{};
	int mod_prefix_index_{};
	std::vector<std::filesystem::path> from_paths_;
	std::string creator_name_;
	std::string main_name_;
	std::string original_link_;
	ModFile* mod_file_handler_{ nullptr };

	/* Extra Mod Params */
	std::filesystem::path to_path_;
	bool use_install_path_{true};

protected:
	ModInfo(const std::string& main_name);
	ModInfo(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& creator_name, const std::string& main_name, const std::string& original_link);

public:
	ModInfo(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& to_path, const std::string& main_name, const bool& use_install_path = true, const std::string& custom_extension = "");

	inline static std::vector<ModInfo*> ModInfoList;

	std::string generate_mod_name();

	static ModInfo* add_mod(const std::string& link, const std::vector<std::filesystem::path>& from_paths, const std::string& to_path, const std::string& main_name, const bool& use_install_path = true, const std::string& custom_extension = "");
	static NosLib::Result<void> parse_modpack_file(const std::filesystem::path& file_path);
	void process_mod();

protected:
	static ModInfo* parse_line(std::string& line);

	void update_loading_screen(const std::string& status);
	void update_loading_screen(const int& percentage);
	void update_loading_screen(const int& percentage, const std::string& status);

	NosLib::Result<void> standard_mod_process();
	NosLib::Result<void> custom_mod_process();
	NosLib::Result<void> separator_mod_process();

	void InitialResponseCallback(const std::string& statusString);
	bool ProgressCallback(uint64_t len, uint64_t total);
};