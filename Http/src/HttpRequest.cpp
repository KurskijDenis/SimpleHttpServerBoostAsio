#include <Http/HttpRequest.hpp>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>


namespace Http
{

namespace
{

constexpr const char* name_value_separator = ": ";
constexpr const char* crlf = "\r\n";

bool IsKeepAliveConnection(const std::string& value)
{
	const ICStringEqual comp;
	return comp(value, "kepp-alive");
}

} // namespace

HttpRequest::HttpRequest(
	const HttpMethodType method,
	std::string uri,
	const HttpVersion& version,
	HeadersMap headers,
	std::string body)
	: method_(method)
	, version_(version)
	, headers_(std::move(headers))
{
	if (method_ == HttpMethodType::Unknown)
	{
		throw std::runtime_error("Unknown http method type");
	}

	SetURI(std::move(uri));
	SetBody(std::move(body));

	const auto header_it = headers_.find("Connection");
	if (header_it != headers_.cend())
	{
		keep_alive_ = IsKeepAliveConnection(header_it->second);
	}
}

HttpMethodType HttpRequest::GetMethodType() const noexcept
{
	return method_;
}

const std::string& HttpRequest::GetURI() const noexcept
{
	return uri_;
}

const HttpVersion& HttpRequest::GetHTTPVersion() const noexcept
{
	return version_;
}

const HeadersMap& HttpRequest::GetHeaders() const noexcept
{
	return headers_;
}

const std::string& HttpRequest::GetBody() const noexcept
{
	return body_;
}

bool HttpRequest::IsKeepAlive() const noexcept
{
	return keep_alive_;
}

std::string HttpRequest::PackToString() const
{
	std::string buffer;
	auto method_name = ConvertToString(method_);
	buffer.reserve(method_name.size() + uri_.size() + 8192 + 12 + body_.size());
	buffer += std::move(method_name) + " ";
	buffer += uri_ + " ";
	buffer += ConvertToString(version_);
	buffer += crlf;
	for (const auto& [key, value] : headers_)
	{
		buffer += key;
		buffer += name_value_separator;
		buffer += value;
		buffer += crlf;
	}
	buffer += crlf;
	buffer += body_;
	return buffer;
}

void HttpRequest::SetURI(std::string uri)
{
	if (uri.empty())
	{
		uri_ = "/";
	}
	else if (uri.front() != '/')
	{
		uri_ = "/" + std::move(uri);
	}
	else
	{
		std::swap(uri_, uri);
	}
}

HttpRequest& HttpRequest::SetHeader(const std::string& key, std::string value)
{
	{
		const ICStringEqual comp;
		if (comp(key, "Connection"))
		{
			keep_alive_ = IsKeepAliveConnection(value);
		}
	}
	std::swap(headers_[key], value);
	return *this;
}

void HttpRequest::SetBody(std::string body)
{
	std::swap(body, body_);
	if (body_.empty())
	{
		headers_.erase("Content-Length");
	}
	else
	{
		SetHeader("Content-Length", std::to_string(body_.size()));
	}
	std::swap(body_, body);
}

} // namespace Http
