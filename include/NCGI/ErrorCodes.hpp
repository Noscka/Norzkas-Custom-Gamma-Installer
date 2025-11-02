#pragma once

#include <system_error>
#include <string>
#include <cstdint>

	enum class NCGIError : uint8_t
	{
		Successful,
		UnknownProvider,
		HttpError,
		ExtractionFailure
	};

	class NCGIErrorCategory : public std::error_category
	{
	public:
		static const NCGIErrorCategory& instance()
		{
			static NCGIErrorCategory inst;
			return inst;
		}

		const char* name() const noexcept override
		{
			return "NCGIErrorCategory";
		}

		std::string message(int ev) const override
		{
			switch (static_cast<NCGIError>(ev))
			{
			case NCGIError::Successful:
				return "Successful";

			case NCGIError::UnknownProvider:
				return "Known mod provider";

			case NCGIError::HttpError:
				return "Error when doing HTTP operations";

			case NCGIError::ExtractionFailure:
				return "Failed to extract";
			}

			return "Unknown error";
		}
	};

	inline std::error_code make_error_code(NCGIError e) noexcept
	{
		return { static_cast<int>(e), NCGIErrorCategory::instance() };
	}

namespace std
{
	template <>
	struct is_error_code_enum<NCGIError> : true_type {};
}
