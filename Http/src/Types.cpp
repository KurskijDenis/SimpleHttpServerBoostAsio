#include <Http/Types.hpp>

#include <algorithm>


namespace Http
{

namespace
{

const std::unordered_map<std::string_view, HttpMethodType>& GetAllowedMethodsNotation() noexcept
{
	static const std::unordered_map<std::string_view, HttpMethodType> allowed_methods = {
		{"GET", HttpMethodType::Get},
		{"POST", HttpMethodType::Post},
		{"PUT", HttpMethodType::Put},
		{"OPTIONS", HttpMethodType::Options},
		{"CONNECT", HttpMethodType::Connect},
		{"HEAD", HttpMethodType::Head},
		{"PATCH", HttpMethodType::Patch},
		{"DELETE", HttpMethodType::Delete},
		{"TRACE", HttpMethodType::Trace},
	};
	return allowed_methods;
}

size_t GetMaxHttpMethodSizeImpl()
{
	const auto& methods = GetAllowedMethodsNotation();
	size_t max_size = 0;
	for (const auto& method : methods)
	{
		max_size = std::max(max_size, method.first.size());
	}
	return max_size;
}

} // namespace

size_t ICStringHash::operator()(const std::string& key) const
{
	std::string converted_key_;
	converted_key_.reserve(key.size());
	std::transform(
		key.cbegin(),
		key.cend(),
		std::back_inserter(converted_key_),
		[](const auto ch) { return std::toupper(ch);});

	const std::hash<std::string> hash;
	return hash(converted_key_);
}

bool ICStringEqual::operator()(const std::string& value1, const std::string& value2) const
{
	const auto str_size = value1.size();
	if (str_size != value2.size())
	{
		return false;
	}
	for (size_t i = 0; i < str_size; ++i)
	{
		if (std::toupper(value1[i]) != std::toupper(value2[i]))
		{
			return false;
		}
	}
	return true;
}

std::string GetDefaultStatusText(const StatusCode status_code)
{
	static const std::unordered_map<StatusCode, std::string> statuses = {
		{StatusCode::Ok, "OK"},
		{StatusCode::Created, "Created"},
		{StatusCode::Accepted, "Accepted"},
		{StatusCode::NoContent, "No Content"},
		{StatusCode::MultipleChoices, "Multiple Choices"},
		{StatusCode::MovedPermanently, "Moved Permanently"},
		{StatusCode::MovedTemporarily, "Moved Temporarily"},
		{StatusCode::NotModified, "Not Modified"},
		{StatusCode::BadRequest, "Bad Request"},
		{StatusCode::Unauthorized, "Unauthorized"},
		{StatusCode::Forbidden, "Forbidden"},
		{StatusCode::NotFound, "Not Found"},
		{StatusCode::InternalServerError, "Internal Server Error"},
		{StatusCode::NotImplemented, "Not Implemented"},
		{StatusCode::BadGateway, "Bad Gateway"},
		{StatusCode::ServiceUnavailable, "Service Unavailable"},
	};
	const auto it = statuses.find(status_code);
	return it != statuses.cend() ? it->second : "Internal Server Error";
}

std::string GetDefaultHtmlText(const StatusCode status_code)
{
	static const std::unordered_map<StatusCode, std::string> statuses = {
		{
			StatusCode::Ok,
			""
		},
		{
			StatusCode::Created,
			"<html>"
			"<head><title>Created</title></head>"
			"<body><h1>201 Created</h1></body>"
			"</html>"
		},
		{
			StatusCode::Accepted,
			"<html>"
			"<head><title>Accepted</title></head>"
			"<body><h1>202 Accepted</h1></body>"
			"</html>"
		},
		{
			StatusCode::NoContent,
			"<html>"
			"<head><title>No Content</title></head>"
			"<body><h1>204 Content</h1></body>"
			"</html>"
		},
		{
			StatusCode::MultipleChoices,
			"<html>"
			"<head><title>Multiple Choices</title></head>"
			"<body><h1>300 Multiple Choices</h1></body>"
			"</html>"
		},
		{
			StatusCode::MovedPermanently,
			"<html>"
			"<head><title>Moved Permanently</title></head>"
			"<body><h1>301 Moved Permanently</h1></body>"
			"</html>"
		},
		{
			StatusCode::MovedTemporarily,
			"<html>"
			"<head><title>Moved Temporarily</title></head>"
			"<body><h1>302 Moved Temporarily</h1></body>"
			"</html>"
		},
		{
			StatusCode::NotModified,
			"<html>"
			"<head><title>Not Modified</title></head>"
			"<body><h1>304 Not Modified</h1></body>"
			"</html>"
		},
		{
			StatusCode::BadRequest,
			"<html>"
			"<head><title>Bad Request</title></head>"
			"<body><h1>400 Bad Request</h1></body>"
			"</html>"
		},
		{
			StatusCode::Unauthorized,
			"<html>"
			"<head><title>Unauthorized</title></head>"
			"<body><h1>401 Unauthorized</h1></body>"
			"</html>"
		},
		{
			StatusCode::Forbidden,
			"<html>"
			"<head><title>Forbidden</title></head>"
			"<body><h1>403 Forbidden</h1></body>"
			"</html>"
		},
		{
			StatusCode::NotFound,
			"<html>"
			"<head><title>Not Found</title></head>"
			"<body><h1>404 Not Found</h1></body>"
			"</html>"
		},
		{
			StatusCode::InternalServerError,
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>500 Internal Server Error</h1></body>"
			"</html>"
		},
		{
			StatusCode::NotImplemented,
			"<html>"
			"<head><title>Not Implemented</title></head>"
			"<body><h1>501 Not Implemented</h1></body>"
			"</html>"
		},
		{
			StatusCode::BadGateway,
			"<html>"
			"<head><title>Bad Gateway</title></head>"
			"<body><h1>502 Bad Gateway</h1></body>"
			"</html>"
		},
		{
			StatusCode::ServiceUnavailable,
			"<html>"
			"<head><title>Service Unavailable</title></head>"
			"<body><h1>503 Service Unavailable</h1></body>"
			"</html>"
		},
	};
	static const std::string default_response = "<html>"
		"<head><title>Internal Server Error</title></head>"
		"<body><h1>500 Internal Server Error</h1></body>"
		"</html>";

	const auto it = statuses.find(status_code);
	return it != statuses.cend() ? it->second : default_response;
}

size_t GetMaxHttpMethodSize()
{
	static const auto max_http_method_size = GetMaxHttpMethodSizeImpl();
	return max_http_method_size;
}

std::string ConvertToString(const HttpVersion& version)
{
	return "HTTP/" + std::to_string(version.major) + "." + std::to_string(version.minor);
}

std::string ConvertToString(const StatusCode status_code)
{
	return std::to_string(static_cast<unsigned>(status_code));
}

std::string ConvertToString(const HttpMethodType method) noexcept
{
	const auto& methods = GetAllowedMethodsNotation();
	const auto it = std::find_if(
		methods.cbegin(),
		methods.cend(),
		[method](const auto& v) { return v.second == method; } );
	return it != methods.cend() ? std::string{it->first} : "Unknown";
}

HttpMethodType GetHttpMethodTypeFromString(const std::string_view str)
{
	const auto& methods = GetAllowedMethodsNotation();
	const auto it = methods.find(str);
	return it != methods.cend() ? it->second : HttpMethodType::Unknown;
}

StatusCode GetStatusCodeFromString(const std::string& status_code_str)
{
	static const std::unordered_map<std::string, StatusCode> statuses = {
		{ConvertToString(StatusCode::Ok), StatusCode::Ok},
		{ConvertToString(StatusCode::Created), StatusCode::Created},
		{ConvertToString(StatusCode::Accepted), StatusCode::Accepted},
		{ConvertToString(StatusCode::NoContent), StatusCode::NoContent},
		{ConvertToString(StatusCode::MultipleChoices), StatusCode::MultipleChoices},
		{ConvertToString(StatusCode::MovedPermanently), StatusCode::MovedPermanently},
		{ConvertToString(StatusCode::MovedTemporarily), StatusCode::MovedTemporarily},
		{ConvertToString(StatusCode::NotModified), StatusCode::NotModified},
		{ConvertToString(StatusCode::BadRequest), StatusCode::BadRequest},
		{ConvertToString(StatusCode::Unauthorized), StatusCode::Unauthorized},
		{ConvertToString(StatusCode::Forbidden), StatusCode::Forbidden},
		{ConvertToString(StatusCode::NotFound), StatusCode::NotFound},
		{ConvertToString(StatusCode::InternalServerError), StatusCode::InternalServerError},
		{ConvertToString(StatusCode::NotImplemented), StatusCode::NotImplemented},
		{ConvertToString(StatusCode::BadGateway), StatusCode::BadGateway},
		{ConvertToString(StatusCode::ServiceUnavailable), StatusCode::ServiceUnavailable},
	};
	const auto it = statuses.find(status_code_str);
	return it != statuses.cend() ? it->second : StatusCode::Unknown;
}

StatusCode GetStatusCodeFromUnsigned(const unsigned status_code)
{
	static const std::unordered_map<unsigned, StatusCode> statuses = {
		{static_cast<unsigned>(StatusCode::Ok), StatusCode::Ok},
		{static_cast<unsigned>(StatusCode::Created), StatusCode::Created},
		{static_cast<unsigned>(StatusCode::Accepted), StatusCode::Accepted},
		{static_cast<unsigned>(StatusCode::NoContent), StatusCode::NoContent},
		{static_cast<unsigned>(StatusCode::MultipleChoices), StatusCode::MultipleChoices},
		{static_cast<unsigned>(StatusCode::MovedPermanently), StatusCode::MovedPermanently},
		{static_cast<unsigned>(StatusCode::MovedTemporarily), StatusCode::MovedTemporarily},
		{static_cast<unsigned>(StatusCode::NotModified), StatusCode::NotModified},
		{static_cast<unsigned>(StatusCode::BadRequest), StatusCode::BadRequest},
		{static_cast<unsigned>(StatusCode::Unauthorized), StatusCode::Unauthorized},
		{static_cast<unsigned>(StatusCode::Forbidden), StatusCode::Forbidden},
		{static_cast<unsigned>(StatusCode::NotFound), StatusCode::NotFound},
		{static_cast<unsigned>(StatusCode::InternalServerError), StatusCode::InternalServerError},
		{static_cast<unsigned>(StatusCode::NotImplemented), StatusCode::NotImplemented},
		{static_cast<unsigned>(StatusCode::BadGateway), StatusCode::BadGateway},
		{static_cast<unsigned>(StatusCode::ServiceUnavailable), StatusCode::ServiceUnavailable},
	};
	const auto it = statuses.find(status_code);
	return it != statuses.cend() ? it->second : StatusCode::Unknown;
}

} // namespace Http
