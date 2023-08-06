#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>


namespace Http
{

/**
 * \brief Supported http methods.
 */
enum class HttpMethodType
{
	Get,
	Post,
	Put,
	Options,
	Connect,
	Head,
	Patch,
	Delete,
	Trace,
	Unknown,
};

/**
 * \brief Supported status codes.
 */
enum class StatusCode : unsigned
{
	Ok = 200,
	Created = 201,
	Accepted = 202,
	NoContent = 204,
	MultipleChoices = 300,
	MovedPermanently = 301,
	MovedTemporarily = 302,
	NotModified = 304,
	BadRequest = 400,
	Unauthorized = 401,
	Forbidden = 403,
	NotFound = 404,
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	Unknown = 10000
};

/**
 * \brief Http version.
 */
struct HttpVersion final
{
	unsigned major = 1;
	unsigned minor = 1;
};

/**
 * \brief Ignore case string hash.
 */
struct ICStringHash
{
	size_t operator()(const std::string& key) const;
};

/**
 * \brief Ignore case string comaparator.
 */
struct ICStringEqual
{
	bool operator()(const std::string& value1, const std::string& value2) const;
};

/**
 * \brief Http headers map.
 */
using HeadersMap = std::unordered_map<std::string, std::string, ICStringHash, ICStringEqual>;

/**
 * \brief Returns default status text by status code.
 */
std::string GetDefaultStatusText(StatusCode status_code);

/**
 * \brief Returns default html page by status code.
 */
std::string GetDefaultHtmlText(StatusCode status_code);

/**
 * \brief Get max http method string size.
 */
size_t GetMaxHttpMethodSize();

/**
 * \brief Convert http version to string.
 */
std::string ConvertToString(const HttpVersion& version);

/**
 * \brief Return string representation for http status code.
 */
std::string ConvertToString(StatusCode status_code);

/**
 * \brief Return string representation for http method.
 */
std::string ConvertToString(HttpMethodType method) noexcept;

/**
 * \brief Return http method by string representation.
 */
HttpMethodType GetHttpMethodTypeFromString(std::string_view status_code_str);

/**
 * \brief Return status code by string representation.
 */
StatusCode GetStatusCodeFromString(const std::string& status_code_str);

/**
 * \brief Return status code by int representation.
 */
StatusCode GetStatusCodeFromUnsigned(const unsigned status_code);

} // namespace Http
