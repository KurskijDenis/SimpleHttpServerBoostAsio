#include <Http/HttpResponse.hpp>

#include <utility>
#include <string>


namespace Http
{

namespace
{

constexpr const char* name_value_separator = ": ";
constexpr const char* crlf = "\r\n";

} // namsespace

HttpResponse::HttpResponse(
	const StatusCode status_code,
	HeadersMap headers,
	std::string body,
	std::string status_text,
	const HttpVersion& http_version)
	: status_code_(status_code)
	, status_text_(!status_text.empty() ? std::move(status_text) : GetDefaultStatusText(status_code_))
	, version_(http_version)
	, headers_(std::move(headers))
{
	SetBody(std::move(body));
}

StatusCode HttpResponse::GetStatusCode() const noexcept
{
	return status_code_;
}

const std::string& HttpResponse::GetStatusText() const noexcept
{
	return status_text_;
}

const HttpVersion& HttpResponse::GetHTTPVersion() const noexcept
{
	return version_;
}

const HeadersMap& HttpResponse::GetHeaders() const noexcept
{
	return headers_;
}

const std::string& HttpResponse::GetBody() const noexcept
{
	return body_;
}

std::string HttpResponse::PackToString() const
{
	std::string buffer;
	buffer.reserve(8192 + 16 + status_text_.size() + body_.size());
	buffer += ConvertToString(version_) + " ";
	buffer += ConvertToString(status_code_) + " ";
	buffer += status_text_;
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

void HttpResponse::SetHttpStatusText(std::string status_text)
{
	std::swap(status_text, status_text_);
}

HttpResponse& HttpResponse::SetHeader(const std::string& key, std::string value)
{
	std::swap(headers_[key], value);
	return *this;
}

void HttpResponse::SetBody(std::string body)
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
}

HttpResponse StockResponse(const StatusCode status_code)
{
	return HttpResponse{
		status_code,
		{{"Content-Type", "text/html"}},
		GetDefaultHtmlText(status_code),
		GetDefaultStatusText(status_code),
		HttpVersion{}
	};
}

} // namespace Http
