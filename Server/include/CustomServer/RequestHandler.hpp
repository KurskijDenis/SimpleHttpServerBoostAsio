#pragma once

#include <string>

namespace Http
{

struct HttpResponse;
struct HttpRequest;

} // namespace Http

namespace Http::Server
{

/**
 * \brief Http handler.
 */
class RequestHandler
{
public:
	RequestHandler(const RequestHandler&) = delete;
	RequestHandler& operator=(const RequestHandler&) = delete;

	RequestHandler(RequestHandler&&) = delete;
	RequestHandler& operator=(RequestHandler&&) = delete;

	explicit RequestHandler(const std::string& doc_root);

	/**
	 * \brief Handle http request.
	 */
	HttpResponse HandleRequest(const HttpRequest& req);

private:
	//! The directory containing the files to be served.
	std::string doc_root_;
};

} // namespace Http::Server
