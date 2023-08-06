#include <CustomServer/HttpRequestConnection.hpp>

#include <CustomServer/Connection.hpp>

#include <Http/HttpResponse.hpp>

#include <stdexcept>


namespace Http::Server
{

HttpRequestConnection::HttpRequestConnection(HttpRequest http_request, ConnectionPtr connection)
	: request_(std::move(http_request))
	, connection_(std::move(connection))
{
	if (!connection_)
	{
		throw std::runtime_error("Can't create HttpRequestConnection without connection");
	}
}

bool HttpRequestConnection::Send(const HttpResponse& msg)
{
	if (response_sended_)
	{
		return false;
	}
	response_sended_ = true;
	return connection_->Write(msg.PackToString(), request_.IsKeepAlive());
}

const HttpRequest& HttpRequestConnection::GetRequest() const noexcept
{
	return request_;
}

bool HttpRequestConnection::IsAlive() const noexcept
{
	return connection_->ConnectionIsAvailable();
}

HttpRequestConnection::~HttpRequestConnection()
{
	if (!response_sended_)
	{
		const auto response = StockResponse(StatusCode::InternalServerError);
		Send(response);
	}
}

} // namespace Http::Server
