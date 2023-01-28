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

  static std::string get(const std::string url)
  {
    URI uri(url);
    HTTPClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathAndQuery(), HTTPResponse::HTTP_1_1);
    request.set("Authorization", "Bearer " + API_KEY);
    request.set("OpenAI-Client-Organization", ORGANIZATION_ID);

    HTTPResponse response;
    session.sendRequest(request);
    std::istream &rs = session.receiveResponse(response);

    std::string r;
    Poco::StreamCopier::copyToString(rs, r);
    return r;
  }
};
