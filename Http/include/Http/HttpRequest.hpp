#pragma once

#include <Http/Types.hpp>

#include <string>
#include <unordered_map>
#include <optional>


namespace Http
{

/**
 * \brief Http request.
 */
class HttpRequest final
{
public:
	HttpRequest(
		HttpMethodType method,
		std::string uri,
		const HttpVersion& version,
		HeadersMap headers,
		std::string body);

	/**
	 * \brief Return http method type.
	 */
	[[nodiscard]] HttpMethodType GetMethodType() const noexcept;
	/**
	 * \brief Return uri.
	 */
	[[nodiscard]] const std::string& GetURI() const noexcept;
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
	 * \brief Return true, if connection should be alive, false otherwise.
	 */
	[[nodiscard]] bool IsKeepAlive() const noexcept;
	/**
	 * \brief Pack http request to string.
	 */
	[[nodiscard]] std::string PackToString() const;
	/**
	 * \brief Set new url.
	 */
	void SetURI(std::string uri);
	/**
	 * \brief Set new header.
	 */
	HttpRequest& SetHeader(const std::string& key, std::string value);
	/**
	 * \brief Set body.
	 */
	void SetBody(std::string body);

private:
	//! Http method type.
	HttpMethodType method_ = HttpMethodType::Unknown;
	//! Http connection is keep alive.
	bool keep_alive_ = false;
	//! Http uri.
	std::string uri_;
	//! Http version.
	HttpVersion version_;
	//! Http headers.
	HeadersMap headers_;
	//! Http body.
	std::string body_;
};

} // namespace Http
