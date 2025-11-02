#include <NCGI/ModDB.hpp>
#include <NCGI/ErrorCodes.hpp>
#include <NosLib/Logging.hpp>
#include <html.hpp>

NosLib::Result<NosLib::URL> ModDB::get_download_url(const NosLib::URL& download_page)
{
	return get_instance().get_mirror_url(download_page);
}

NosLib::Result<NosLib::URL> ModDB::get_mirror_url(const NosLib::URL& download_page)
{
	NosLib::Result<std::string> page_content_res = get_page_content(download_page);
	if ( !page_content_res)
	{
		return {page_content_res.ErrorCode(), page_content_res.GetAdditionalErrorMessage()};
	}
	std::string page_content = *page_content_res;

	html::parser p;
	html::node_ptr node = p.parse(page_content);

	std::vector<html::node*> mirror_containers = node->select("a[href]");

	if (mirror_containers.size() < 1)
	{
		return {NCGIError::HttpError, "didn't find mirror links"};
	}

	return NosLib::URL(mirror_containers[0]->get_attr("href"));
}

NosLib::Result<std::string> ModDB::get_page_content(const NosLib::URL& download_page)
{
	httplib::Result res = ModDBMirrorClient->Get(download_page.GetSubdir());

	if (!res)
	{
		return{ NCGIError::HttpError, httplib::to_string(res.error()) };
	}

	if (res->status == 503) /* TODO: REPLACE WITH ACTUAL CODES */
	{
		return{ NCGIError::HttpError, "Too many requests" };
	}

	if (res->status != 200)
	{
		return { NCGIError::HttpError, std::format("unexpected status {} | reason: {}", res->status, res->reason) };
	}

	return res->body;
}