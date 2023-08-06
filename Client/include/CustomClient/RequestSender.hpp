#pragma once

#include <CustomClient/ClientState.hpp>

#include <Http/HttpRequest.hpp>
#include <Http/HttpResponse.hpp>

#include <boost/asio.hpp>

#include <cstdint>
#include <chrono>
#include <functional>
#include <optional>
#include <thread>
#include <string>


namespace Http::Client
{

/**
 * \brief Send and control requests.
 */
class RequestSender final
{
public:
	RequestSender(const RequestSender&) = delete;
	RequestSender(RequestSender&&) = delete;

	RequestSender& operator=(const RequestSender&) = delete;
	RequestSender& operator=(RequestSender&&) = delete;

	explicit RequestSender(
		std::string address = "127.0.0.1",
		uint16_t port = 80,
		const std::chrono::seconds& timeout = std::chrono::seconds{60});

	/**
	 * \brief Send http request.
	 */
	void SendRequest(
		const HttpRequest& http_request,
		std::function<void(std::optional<HttpResponse>)> response_handler,
		std::optional<std::string> address = std::nullopt,
		std::optional<uint16_t> port = std::nullopt,
		std::optional<std::chrono::seconds> timeout = std::nullopt);

	/**
	 * \brief Stop all requests.
	 */
	void Stop();

	~RequestSender();

private:
	//! Boost asio context.
	boost::asio::io_context io_context_;
	//! The work-tracking executor that keep the io_contexts running.
	boost::asio::any_io_executor work_;
	//! Address.
	const std::string address_ = "127.0.0.1";
	//! Working thread.
	std::optional<std::thread> work_thread_;
	//! Port.
	const uint16_t port_ = 80;
	//! Connection timeout.
	const std::chrono::seconds timeout_{0};
	//! Client requests state.
	State state_;
};

} // namespace Http::Client
