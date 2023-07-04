#include "request/response.hh"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <istream>
#include <regex>
#include <sstream>
#include <string>

#include "misc/file_to_string.hh"
#include "request/error.hh"
#include "request/types.hh"

namespace http
{
    static std::string get_date_header()
    {
        std::ostringstream iss;
        auto t = std::time(nullptr);
        iss << "Date: "
            << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT")
            << http_crlf;
        return iss.str();
    }

    static void erase_substr(std::string &str, const std::string &root)
    {
        size_t pos = str.find(root);
        if (pos != std::string::npos)
        {
            str.erase(pos, root.length());
        }
    }

    static std::string get_directory_entries(const std::filesystem::path &path,
                                             const std::string &root)
    {
        std::cout << "The Path is : " << path << std::endl;
        std::string entries = "";
        std::string path_str = path.string();
        erase_substr(path_str, root);
        if (path_str != "/")
            path_str.push_back('/');
        entries += "<li><a href=\"" + path_str + "..\">..</a></li>\n";
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            std::string entry_path = entry.path().string();
            std::string entry_filename = entry.path().filename().string();
            erase_substr(entry_path, root);
            std::cout << "The entry is :" << entry_path << std::endl;
            std::cout << "The entry name is : " << entry_filename << std::endl;
            entries += "<li><a href=\"" + entry_path + "\">" + entry_filename
                + "</a></li>\n";
        }
        return entries;
    }

    static std::string response_body_get(const std::string &target,
                                         bool auto_index,
                                         const std::string &root)
    {
        std::string body;
        if (!auto_index)
            return file_to_string(target);
        else
        {
            std::filesystem::path path(target);
            std::error_code ec;
            if (std::filesystem::is_directory(path, ec))
            {
                std::string target_mod = target;
                erase_substr(target_mod, root);
                body = "<!DOCTYPE html>\n<html>\n<head>\n<meta "
                       "charset=utf-8>\n<title>Index of "
                    + target_mod + "</title>\n</head>\n<body>\n<ul>\n";
                body += get_directory_entries(path, root);
                body += "</ul>\n</body>\n</html>\n";
            }
            else
            {
                return file_to_string(target);
            }
            if (ec)
            {
                throw std::runtime_error("Error: cannot open directory");
            }
        }
        return body;
    }

    static std::string generic_head_get(const Request &request, bool is_get)
    {
        auto response = std::string{ "HTTP/1.1 200 OK\r\n" };
        response.append(get_date_header());
        auto body = response_body_get(request.target, request.auto_index,
                                      request.vhost_root);
        std::stringstream headers;
        std::string connection_str =
            request.connect == connection::CLOSE ? "close" : "keep-alive";
        headers << "Content-Length: " << body.size()
                << "\r\nConnection: " << connection_str << "\r\n\r\n";
        response.append(headers.str());
        if (is_get)
            response.append(body);
        return response;
    }

    static std::string response_from_get(const Request &request)
    {
        return generic_head_get(request, true);
    }

    static std::string response_from_head(const Request &request)
    {
        return generic_head_get(request, false);
    }

    static std::string response_from_post(const Request &request)
    {
        // We handle POST requests as if they were GET requests
        return response_from_get(request);
    }

    Response::Response(const Request &request)
    {
        switch (request.method)
        {
        case Request::GET:
            this->str = response_from_get(request);
            break;
        case Request::HEAD:
            this->str = response_from_head(request);
            break;
        case Request::POST:
            this->str = response_from_post(request);
            break;
        case Request::NOT_ALLOW:
        default:
            break;
        }
        this->connect = request.connect;
    }

    static std::string
    generic_error_response(const STATUS_CODE &code,
                           const std::string &reason_phrase,
                           const std::string &version = "HTTP/1.1")
    {
        auto response = std::stringstream{};
        response << version << " " << code << " " << reason_phrase << http_crlf;
        auto str = response.str();
        str.append(get_date_header());
        str.append("Content-Length: 0\r\nConnection: close\r\n");
        return str;
    }

    Response::Response(const STATUS_CODE &code,
                       const std::string &reason_phrase,
                       const std::vector<std::string> &extra_fields)
    {
        this->connect = connection::CLOSE;
        this->str = generic_error_response(code, reason_phrase);
        for (const auto &field : extra_fields)
        {
            this->str.append(field);
            this->str.append(http_crlf);
        }
        this->str.append(http_crlf);
    }

    Response::Response(const Request &request, const STATUS_CODE &code,
                       const std::string &reason_phrase,
                       const std::vector<std::string> &extra_fields)
    {
        this->connect = request.connect;
        if (code == STATUS_CODE::INTERNAL_SERVER_ERROR)
            this->connect = connection::CLOSE;
        this->str = generic_error_response(code, reason_phrase);
        for (const auto &field : extra_fields)
        {
            this->str.append(field);
            this->str.append(http_crlf);
        }
        this->str.append(http_crlf);
    }

    Response::Response(const Request &request, const STATUS_CODE &code,
                       const std::string &reason_phrase)
    {
        this->connect = request.connect;
        if (code == STATUS_CODE::BAD_REQUEST)
            this->connect = connection::CLOSE;
        this->str =
            generic_error_response(code, reason_phrase, request.version);
        if (code == STATUS_CODE::METHOD_NOT_ALLOWED)
            this->str.append("Allow: GET, POST, HEAD\r\n");
        else if (code == STATUS_CODE::UPGRADE_REQUIRED)
            this->str.append("Upgrade: HTTP/1.1\r\n");
        this->str.append(http_crlf);
    }

    Response::Response(const STATUS_CODE &code,
                       const std::string &reason_phrase)
    {
        this->str = generic_error_response(code, reason_phrase);
        this->str.append(http_crlf);
    }

    void Response::parse_status_line(const std::string &line)
    {
        auto sstream = std::stringstream(line);
        auto token = std::string{};
        std::getline(sstream, token, ' ');
        this->version = token;
        std::getline(sstream, token, ' ');
        this->status_code = static_cast<STATUS_CODE>(std::stoi(token));
        std::getline(sstream, token);
        this->phrase = token;
    }

    static void trim_string(std::string &str)
    {
        str.erase(0, str.find_first_not_of(" \t"));
        str.erase(str.find_last_not_of(" \t") + 1);
    }

    void Response::parse_header_line(const std::string &line)
    {
        if (line.find(':') == std::string::npos)
            throw std::runtime_error("bad request: no colon");
        auto iss = std::istringstream(line);
        auto field = std::string{};
        std::getline(iss, field, ':');
        if (field.find(' ') != std::string::npos)
            throw std::runtime_error("bad request: space in field");
        auto value = std::string{};
        std::getline(iss, value);
        trim_string(value);
        this->header_fields.insert(std::make_pair(field, value));
    }

    static void check_ends_with_CR(std::string &line)
    {
        if (line.back() != '\r')
            throw std::runtime_error("bad request: no carriage return");
        line.pop_back();
    }

    Response::Response(const std::string header)
    {
        auto iss = std::istringstream(header);
        auto line = std::string{};
        std::getline(iss, line, '\n');
        check_ends_with_CR(line);
        this->parse_status_line(line);
        while (std::getline(iss, line, '\n'))
        {
            check_ends_with_CR(line);
            this->parse_header_line(line);
        }
        auto cl = header_fields.find("Content-Length");
        auto regex = std::regex("0|[1-9][0-9]*");
        if (cl != header_fields.end() && !std::regex_match(cl->second, regex))
            throw std::runtime_error("bad request: Content-Length not valid");
    }

    std::string Response::to_string() const
    {
        auto oss = std::ostringstream{};
        oss << this->version << ' ' << static_cast<uint16_t>(this->status_code)
            << ' ' << this->phrase << http_crlf;
        for (const auto &field_val : this->header_fields)
            oss << field_val.first << ": " << field_val.second << http_crlf;
        oss << http_crlf << this->body;
        return oss.str();
    }
} // namespace http
