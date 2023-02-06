#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <string>

class OpenAI
{
public:
  static std::string getCompletions(std::string &transcript)
  {
    const auto endpoint = baseUrl_ + "/engines/davinci/completions?text=" + transcript;
    return get(endpoint);
  }

private:
  static const std::string baseUrl_ = "https://api.openai.com/v1";
  static const std::string API_KEY = std::getenv("OPENAI_API_KEY");
  static const std::string ORGANIZATION_ID = std::getenv("OPENAI_ORGANIZATION_ID");
  static HTTPClientSession session_;
  static HTTPRequest request_;

  static std::string get(const std::string url)
  {
    URI uri(url);
    request_.setURI(uri.getPathAndQuery());
    request_.set("Authorization", "Bearer " + API_KEY);
    request_.set("OpenAI-Client-Organization", ORGANIZATION_ID);

    HTTPResponse response;
    session_.sendRequest(request_);
    std::istream &rs = session_.receiveResponse(response);

    std::string r;
    Poco::StreamCopier::copyToString(rs, r);
    return r;
  }
};

HTTPClientSession OpenAI::session_("api.openai.com", 443);
HTTPRequest OpenAI::request_(HTTPRequest::HTTP_GET, "/v1", HTTPResponse::HTTP_1_1);
