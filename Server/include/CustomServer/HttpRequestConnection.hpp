#pragma once

#include <Http/HttpRequest.hpp>

#include <memory>
#include <string>


namespace Http
{

class HttpResponse;

} // namespace Http

namespace Http::Server
{

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

/**
 * \brief Hold http connection and can send responce.
 */
class HttpRequestConnection final
{
public:
	HttpRequestConnection(HttpRequest http_request, ConnectionPtr connection);

	HttpRequestConnection(const HttpRequestConnection&) = delete;
	HttpRequestConnection& operator=(const HttpRequestConnection&) = delete;

	HttpRequestConnection(HttpRequestConnection&&) = delete;
	HttpRequestConnection& operator=(HttpRequestConnection&&) = delete;

	/**
	 * \brief Send responce async (not thread safe).
	 *
	 * \param[in] msg Msg to send.
	 *
	 * \return True if can add msg, false otherwise.
	 */
	bool Send(const HttpResponse& msg);

	/**
	 * \brief Return http request.
	 */
	[[nodiscard]] const HttpRequest& GetRequest() const noexcept;

	/**
	 * \brief Check if connection is alive.
	 */
	[[nodiscard]] bool IsAlive() const noexcept;

	~HttpRequestConnection();

private:
	//! Flag, that http request has already sended.
	bool response_sended_ = false;
	//! Http request.
	HttpRequest request_;
	//! Pointer to connection.
	ConnectionPtr connection_;
};

using HttpRequestConnectionUPtr = std::unique_ptr<HttpRequestConnection>;

} // namespace Http::Server
