#pragma once

#include "RequestHandler.hpp"

HttpResponse handleCgi(const HttpRequest &req,
					   const LocationConfig &loc,
					   const std::string &ext);
void freeEnvp(char **envp, size_t size);
HttpResponse parseCgiOutput(const std::vector<unsigned char> &output);
bool isCgiRequest(const HttpRequest &req, const LocationConfig &loc);