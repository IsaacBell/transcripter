#include <iostream>
#include <string>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>

/*
  auto jira = Jira()
      .setSummary("Summary")
      .setDescription("Description")
      .setProjectKey("ProjectKey")
      .setTicketKey("TEST-123")
      .createTicket();
  }

*/
class Jira
{
public:
  Jira() : auth_(std::getenv("JIRA_USERNAME"), std::getenv("JIRA_PASSWORD"));
  {
  }

  Jira &setSummary(std::string &summary)
  {
    summary_ = summary;
    return *this;
  };
  Jira &setDescription(std::string &description)
  {
    description_ = description;
    return *this;
  };
  Jira &setProjectKey(std::string &project_key)
  {
    project_key_ = project_key;
    return *this;
  };
  Jira &setTicketKey(std::string &ticket_key)
  {
    ticket_key_ = ticket_key;
    return *this;
  };

  bool createTicket()
  {
    return createTicketInternal();
  }

  bool checkTicket()
  {
    return ticketExists();
  }

private:
  std::string summary_;
  std::string description_;
  std::string project_key_;
  std::string ticket_key_;
  Poco::Net::HTTPCredentials auth_;
  const std::string baseUrl_ = "https://yourcompany.atlassian.net/rest/api/2/issue/";

  bool createTicketInternal()
  {
    Poco::URI uri(baseUrl_);
    // TODO - need to close the session on delete?
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
    request.setContentType("application/json");

    request.setCredentials(auth_);

    std::string body = "{\"fields\": {\"project\": {\"key\": \"" + project_key_ + "\"},\"summary\": \"" + summary_ + "\",\"description\": \"" + description_ + "\",\"issuetype\": {\"name\": \"Task\"}}}";
    request.setContentLength(body.length());
    std::ostream &request_stream = session.sendRequest(request);
    request_stream << body;

    Poco::Net::HTTPResponse response;
    std::istream &response_stream = session.receiveResponse(response);

    return response.getStatus() == Poco::Net::HTTPResponse::HTTP_CREATED ||
           response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK;
  }

  bool ticketExists()
  {
    try
    {
      Poco::URI uri(baseUrl_ + ticket_key_);
      Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);

      Poco::request.setCredentials(auth_);

      session.sendRequest(request);

      Poco::Net::HTTPResponse response;
      std::istream &response_stream = session.receiveResponse(response);

      return response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK;
    }
    catch (Poco::Exception &e)
    {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }
};
