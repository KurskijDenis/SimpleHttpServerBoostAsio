#include <CustomServer/RequestHandler.hpp>

#include <Http/HttpResponse.hpp>
#include <Http/HttpRequest.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>


namespace Http::Server
{

namespace
{

std::string GetTypeByExt(const std::string_view extension)
{
	static const std::unordered_map<std::string_view, std::string_view> types = {
		{".gif", "image/gif"},
		{".htm", "text/html"},
		{".html", "text/html"},
		{".jpg", "image/jpg"},
		{".png", "image/png"},
		{".json", "application/json"},
		{".txt", "text/plain"},
	};
	const auto it = types.find(extension);
	return it != types.cend() ? std::string{it->second} : std::string{"text/plain"};
}

std::optional<std::string> UriDecode(const std::string& uri)
{
	std::string result;
	result.reserve(uri.size());
	for (std::size_t i = 0; i < uri.size(); ++i)
	{
		if (uri[i] == '+')
		{
			result += ' ';
			continue;
		}
		if (uri[i] != '%')
		{
			result += uri[i];
			continue;
		}
		if (i + 3 > uri.size())
		{
			return std::nullopt;
		}

		int value = 0;
		std::istringstream is(uri.substr(i + 1, 2));
		if (is >> std::hex >> value)
		{
			result += static_cast<char>(value);
			i += 2;
		}
		else
		{
			return std::nullopt;
		}
	}
	return result;
}

} // namespace

RequestHandler::RequestHandler(const std::string& doc_root)
	: doc_root_(doc_root)
{
}

HttpResponse RequestHandler::HandleRequest(const HttpRequest& http_req)
{
	// Decode url to path.
	auto request_path = UriDecode(http_req.GetURI());
	if (!request_path)
	{
		return StockResponse(StatusCode::BadRequest);
	}

	if (!request_path->empty() && request_path->front() == '/')
	{
		*request_path = {request_path->cbegin() + 1, request_path->cend()};
	}
	std::cout << *request_path << std::endl;

	try
	{
		const auto absolute_root_path = std::filesystem::absolute(std::filesystem::path(doc_root_));
		const auto absolute_path = absolute_root_path / std::filesystem::path(*request_path);

		const auto root_path_str = absolute_root_path.string();
		const auto path_str = absolute_path.string();

		if (root_path_str.size() > path_str.size())
		{
			return StockResponse(StatusCode::NotFound);
		}

		if (!root_path_str.empty())
		{
			const auto [it1, it2] = std::mismatch(path_str.cbegin(), path_str.cend(), root_path_str.cbegin(), root_path_str.cend());

			if (it2 != root_path_str.cend())
			{
				std::cout << "Not sub path" << std::endl;
				return StockResponse(StatusCode::NotFound);
			}
		}
		HttpResponse rep{StatusCode::Ok};
		std::string extension;
		if (std::filesystem::is_directory(absolute_path))
		{
			extension = ".html";
			std::string htmp_to_return = "<html>";
			htmp_to_return += "<head><title> Index of " + *request_path + "</title></head>\n<body>\n";
			htmp_to_return += "<h1>Index of " + *request_path + "</h1><hr><pre>\n";
			if (path_str.size() != root_path_str.size())
			{
				htmp_to_return += "<a href=\"../\">../</a>\n";
			}

			for (const auto& entry : std::filesystem::directory_iterator(absolute_path))
			{
				const auto& path = entry.path();;
				const auto filename = path.filename().string();
				htmp_to_return += "<a href=\"" + filename + "\">" + filename + "</a>\n";
			}
			htmp_to_return += "</pre><hr></body>\n</html>";
			rep.SetBody(std::move(htmp_to_return));
		}
		else if (std::filesystem::is_regular_file(absolute_path))
		{
			extension = absolute_path.extension().string();
			std::ifstream is(absolute_path, std::ios::in | std::ios::binary);
			if (!is)
			{
				return StockResponse(StatusCode::NotFound);
			}

			// Fill out the reply to be sent to the client.
			is.seekg(0, is.end);
			const size_t file_size = is.tellg();
			is.seekg(0, is.beg);

			std::string content;
			content.resize(file_size);
			is.read(content.data(), file_size);
			rep.SetBody(std::move(content));
		}
		else
		{
			return StockResponse(StatusCode::NotFound);
		}

		if (http_req.IsKeepAlive())
		{
			rep.SetHeader("Connection", "keep-alive");
		}
		rep.SetHeader("Content-Type", GetTypeByExt(extension));

		return rep;
	}
	catch (const std::exception& exc)
	{
		std::cout << "Got exception " << std::endl;
		return StockResponse(StatusCode::InternalServerError);
	}
	return StockResponse(StatusCode::InternalServerError);
}

} // namespace Http::Server
