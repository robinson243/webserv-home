/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mknroro <mknroro@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:54 by romukena          #+#    #+#             */
/*   Updated: 2026/06/24 01:47:31 by mknroro          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>

static std::string trim(const std::string &s) {
	size_t start = 0;
	while (start < s.size()
		   && std::isspace(static_cast<unsigned char>(s[start])))
		++start;

	size_t end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;

	return s.substr(start, end - start);
}

static std::string toLowerStr(std::string s) {
	std::transform(
		s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::tolower));
	return s;
}

HttpRequest::HttpRequest() : _isValid(false), _code(-1) {
}

HttpRequest::~HttpRequest() {
}

int HttpRequest::getCode() const {
	return _code;
}

std::vector<unsigned char> HttpRequest::getBody() const {
	return _body;
}

bool HttpRequest::getValid() const {
	return _isValid;
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
	return _headers;
}

const std::map<std::string, std::string> &HttpRequest::getRequest() const {
	return _requestLine;
}

void HttpRequest::addBody(std::string &element) {
	_body.insert(_body.end(), element.begin(), element.end());
}

void HttpRequest::makeTrue() {
	_isValid = true;
}

void HttpRequest::addHeaders(const std::string &key, std::string &element) {
	_headers.insert(std::pair<std::string, std::string>(key, element));
}

void HttpRequest::addRequest(const std::string &key, std::string &element) {
	_requestLine.insert(std::pair<std::string, std::string>(key, element));
}

void HttpRequest::print() const {
	std::cout << "=== REQUEST LINE ===" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it =
			 _requestLine.begin();
		 it != _requestLine.end();
		 ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "=== HEADERS ===" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it =
			 _headers.begin();
		 it != _headers.end();
		 ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "=== BODY ===" << std::endl;
	std::cout << std::string(_body.begin(), _body.end()) << std::endl;

	std::cout << "=== STATUS ===" << std::endl;
	std::cout << "code: " << _code << std::endl;
	std::cout << "valid: " << _isValid << std::endl;
}

size_t HttpRequest::requestLength(std::string &e) {
	std::stringstream str(e);
	std::string token;
	size_t i = 0;
	while (str >> token) {
		i++;
	}
	return i;
}

void HttpRequest::addRequestLine(std::stringstream &str) {
	std::string line;
	std::getline(str, line);
	std::stringstream s(line);
	std::string token;
	size_t i = 0;
	if (requestLength(line) != 3) {
		_code = 400;
		return;
	}
	while (s >> token) {
		if (i == 0) {
			if (token != "GET" && token != "POST" && token != "DELETE"
				&& token != "HEAD") {
				_code = 501;
				return;
			}
			addRequest("method", token);
		} else if (i == 1)
			addRequest("uri", token);
		else if (i == 2) {
			if (token != "HTTP/1.1") {
				_code = 505;
				return;
			}
			addRequest("version", token);
		}
		i++;
	}
}

void HttpRequest::substractAndAdd(std::string &token) {
	size_t pos = token.find(':');
	if (pos == std::string::npos || pos == 0) {
		_code = 400;
		return;
	}

	std::string key = trim(token.substr(0, pos));
	std::string value = trim(token.substr(pos + 1));

	key = toLowerStr(key);
	_headers[key] = value;
}

void HttpRequest::addAllHeaders(std::stringstream &str) {
	std::string token;

	while (std::getline(str, token)) {
		if (!token.empty() && token[token.size() - 1] == '\r')
			token.erase(token.size() - 1);

		if (token.empty())
			break;

		if (token.find(':') == std::string::npos || token.find(':') == 0) {
			_code = 400;
			return;
		}

		substractAndAdd(token);
	}
}

bool HttpRequest::findHostInHeaders() {
	const std::map<std::string, std::string> &headers = getHeaders();

	std::map<std::string, std::string>::const_iterator it =
		headers.find("host");

	return it != headers.end() && !it->second.empty();
}

bool HttpRequest::isNumber(std::string &e) {
	for (size_t i = 0; e[i]; i++) {
		if (!isdigit(e[i])) {
			return false;
		}
	}
	return true;
}

bool HttpRequest::isChunked() const {
	std::map<std::string, std::string>::const_iterator it =
		_headers.find("transfer-encoding");

	if (it == _headers.end())
		return false;

	return it->second == "chunked";
}

bool HttpRequest::validateBody(std::string &e) {
	// Si chunked, le body est déjà décodé, pas besoin de vérifier
	// Content-Length
	if (isChunked())
		return true;

	const std::map<std::string, std::string> &headers = getHeaders();
	std::map<std::string, std::string>::const_iterator it =
		headers.find("content-length");

	// Pas de Content-Length = pas de body attendu, c'est OK
	if (it == headers.end())
		return true;

	std::string contentLength = it->second;
	if (!isNumber(contentLength))
		return false;

	char *pEnd;
	long numContentLength = strtol(contentLength.c_str(), &pEnd, 10);

	if (numContentLength < 0) {
		_code = 400;
		return false;
	}

	if (numContentLength != static_cast<long>(e.length()))
		return false;

	return true;
}
std::string HttpRequest::decodeChunkedBody(std::stringstream &str) {
	std::string result;
	std::string sizeLine;

	while (std::getline(str, sizeLine)) {
		if (!sizeLine.empty() && sizeLine[sizeLine.size() - 1] == '\r')
			sizeLine.erase(sizeLine.size() - 1);

		if (sizeLine.empty())
			continue;

		char *pEnd;
		long chunkSize = std::strtol(sizeLine.c_str(), &pEnd, 16);


		if (*pEnd != '\0' || chunkSize < 0) {
			_code = 400;
			return "";
		}

		if (chunkSize == 0) {
			std::string endLine;
			std::getline(str, endLine);
			break;
		}

		std::string chunkData;
		chunkData.resize(chunkSize);

		std::streamsize totalRead = 0;
		while (totalRead < chunkSize && str.good()) {
			str.read(&chunkData[totalRead], chunkSize - totalRead);
			std::streamsize n = str.gcount();


			if (n <= 0) {
				_code = 400;
				return "";
			}
			totalRead += n;
		}

		if (totalRead != chunkSize) {
			_code = 400;
			return "";
		}

		result.append(chunkData);

		char crlf[2];
		str.read(crlf, 2);
		if (str.gcount() != 2 || crlf[0] != '\r' || crlf[1] != '\n') {
			_code = 400;
			return "";
		}
	}

	return result;
}

static std::string toLowerCopy(std::string s) {
	for (size_t i = 0; i < s.size(); ++i)
		s[i] = std::tolower(s[i]);
	return s;
}

bool HttpRequest::isRawRequestComplete(const std::string &req) {
	size_t headerEnd = req.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		return false;
	}

	std::string headers = toLowerCopy(req.substr(0, headerEnd + 4));
	std::string body = req.substr(headerEnd + 4);

	if (headers.find("transfer-encoding: chunked") != std::string::npos) {
		size_t pos = 0;

		while (true) {
			size_t lineEnd = body.find("\r\n", pos);
			if (lineEnd == std::string::npos) {
				return false;
			}

			std::string sizeLine = body.substr(pos, lineEnd - pos);
			size_t semi = sizeLine.find(';');
			if (semi != std::string::npos)
				sizeLine = sizeLine.substr(0, semi);

			char *endptr = NULL;
			long chunkSize = std::strtol(sizeLine.c_str(), &endptr, 16);


			if (*endptr != '\0' || chunkSize < 0)
				return false;

			pos = lineEnd + 2;

			if (chunkSize == 0) {
				if (body.size() >= pos + 2
					&& body.compare(pos, 2, "\r\n") == 0) {
					return true;
				}

				size_t trailerEnd = body.find("\r\n\r\n", pos);
				if (trailerEnd != std::string::npos) {
					return true;
				}

				return false;
			}

			if (body.size() < pos + static_cast<size_t>(chunkSize) + 2) {
				return false;
			}

			pos += chunkSize;

			if (body.compare(pos, 2, "\r\n") != 0) {
				return false;
			}

			pos += 2;
		}
	}

	size_t clPos = headers.find("content-length:");
	if (clPos != std::string::npos) {
		size_t start = clPos + 15;
		while (start < headers.size()
			   && (headers[start] == ' ' || headers[start] == '\t'))
			++start;
		size_t end = headers.find("\r\n", start);
		std::string value = headers.substr(start, end - start);
		long contentLength = std::strtol(value.c_str(), NULL, 10);


		return contentLength >= 0
			   && body.size() >= static_cast<size_t>(contentLength);
	}

	return true;
}

void HttpRequest::addHttpRequest(std::string &req) {

	if (!isRawRequestComplete(req)) {
		return;
	}
	std::stringstream str(req);
	addRequestLine(str);
	if (_code != -1)
		return;

	addAllHeaders(str);
	if (_code != -1)
		return;
	for (std::map<std::string, std::string>::const_iterator it =
			 _headers.begin();
		 it != _headers.end();
		 ++it)
	if (!findHostInHeaders()) {
		_code = 400;
		return;
	}

	std::string body;
	if (isChunked()) {
		body = decodeChunkedBody(str);
	} else {
		// Lire tout le reste du stream
		std::ostringstream oss;
		oss << str.rdbuf();
		body = oss.str();
	}

	if (!validateBody(body)) {
		_code = 400;
		return;
	}

	addBody(body);
	if (_code == -1) {
		_code = 200;
		makeTrue();
	}
}