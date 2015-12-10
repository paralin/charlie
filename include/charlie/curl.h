#pragma once

#define CURL_STATICLIB
#define CURL_INSECURE_HTTPS
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>

static size_t data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
  if(userp)
  {
    std::ostream& os = *static_cast<std::ostream*>(userp);
    std::streamsize len = size * nmemb;
    if(os.write(static_cast<char*>(buf), len))
      return len;
  }

  return 0;
}

CURLcode curl_read(const std::string& url, std::ostream& os, long* status_code = NULL, struct curl_slist * headers = NULL, long timeout = 30)
{
  CURLcode code(CURLE_FAILED_INIT);
  CURL* curl = curl_easy_init();

  if(curl)
  {
    if(CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str()))
#ifdef CURL_INSECURE_HTTPS
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0))
#endif
#ifdef VERBOSE
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L))
#endif
        && (headers == NULL || CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers))))
    {
      code = curl_easy_perform(curl);
      if(status_code)
      {
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, status_code);
      }
    }
    curl_easy_cleanup(curl);
  }
  return code;
}
