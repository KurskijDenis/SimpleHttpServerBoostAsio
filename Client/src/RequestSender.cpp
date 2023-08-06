#include <CustomClient/RequestSender.hpp>

#include <CustomClient/Request.hpp>

#include <iostream>

namespace Http::Client
{

RequestSender::RequestSender(
	std::string address,
	const uint16_t port,
	const std::chrono::seconds& timeout)
	: work_(
		boost::asio::require(
			io_context_.get_executor(),
			boost::asio::execution::outstanding_work.tracked))
	, address_(address)
	, port_(port)
	, timeout_(timeout)
{
	work_thread_.emplace(
		[&io_context = io_context_]()
		{
			io_context.run();
		}
	);
}

void RequestSender::SendRequest(
	const HttpRequest& http_request,
	std::function<void(std::optional<HttpResponse>)> response_handler,
	std::optional<std::string> address,
	std::optional<uint16_t> port,
	std::optional<std::chrono::seconds> timeout)
{
	// Race maybe, but at the end of aplication it isn't very crucial.
	if (state_.IsStopped())
	{
		return;
	}

	auto request = Request::Create(
		io_context_,
		http_request,
		std::move(response_handler),
		address ? std::move(*address) : address_,
		port ? *port : port_,
		timeout ? *timeout : timeout_);

	request->Send();
}

void RequestSender::Stop()
{
	if (!state_.Stop())
	{
		return;
	}
	if (!io_context_.stopped())
	{
		io_context_.stop();
	}
	if (work_thread_ && work_thread_->joinable())
	{
		work_thread_->join();
		work_thread_.reset();
	}
}

RequestSender::~RequestSender()
{
	Stop();
}

} // namespace Http::Client
