#pragma once

#include <Http/Types.hpp>

#include <string>
#include <unordered_map>
#include <optional>


namespace Http
{

/**
 * \brief Http response.
 */
class HttpResponse final
{
public:
	HttpResponse(
		StatusCode status_code,
		HeadersMap headers = {},
		std::string body = {},
		std::string status_text = {},
		const HttpVersion& http_version = {});

	/**
	 * \brief Return http status code.
	 */
	[[nodiscard]] StatusCode GetStatusCode() const noexcept;
	/**
	 * \brief Return http status text.
	 */
	[[nodiscard]] const std::string& GetStatusText() const noexcept;
	/**
	 * \brief Return http version.
	 */
	[[nodiscard]] const HttpVersion& GetHTTPVersion() const noexcept;
	/**
	 * \brief Return http headers.
	 */
	[[nodiscard]] const HeadersMap& GetHeaders() const noexcept;
	/**
	 * \brief Return http body.
	 */
	[[nodiscard]] const std::string& GetBody() const noexcept;
	/**
	 * \brief Pack http response to string.
	 */
	[[nodiscard]] std::string PackToString() const;
	/**
	 * \brief Pack http response to string.
	 */
	void SetHttpStatusText(std::string status_text);
	/**
	 * \brief Set header key value.
	 */
	HttpResponse& SetHeader(const std::string& key, std::string value);
	/**
	 * \brief Set body.
	 */
	void SetBody(std::string body);

private:
	//! Http status code.
	StatusCode status_code_ = StatusCode::Unknown;
	//! Http status text.
	std::string status_text_;
	//! Http version.
	HttpVersion version_;
	//! Http headers.
	HeadersMap headers_;
	//! Http body.
	std::string body_;
};

/**
 * \brief Return default response by status code.
 */
HttpResponse StockResponse(StatusCode status_code);

} // namespace Http
