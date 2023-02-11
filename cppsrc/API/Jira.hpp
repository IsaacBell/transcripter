#include <map>
#include <iostream>
#include <string>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>

#include "spdlog/spdlog.h"

/*
  auto jira = Jira()
      .setSummary("Summary")
      .setDescription("Description")
      .setIssueType("Bug")
      .setProjectKey("ProjectKey")
      .setTicketKey("TEST-123")
      .createTicket();
  }
*/

enum class JiraIssueType : char
{
  Task,
  Bug,
  Epic,
  Story,
  Improvement,
  NewFeature,
  Subtask,
  TechnicalTask,
  Test
};

JiraIssueType issueType;

class Jira
{
public:
  Jira() : auth_(std::getenv("JIRA_USERNAME"), std::getenv("JIRA_PASSWORD")) {}

  bool createTicket() { return createTicketInternal(); }
  bool checkTicket() { return ticketExists(); }
  bool isValid() { return isValidIssueType(); }

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
  Jira &setProjectKey(std::string &projectKey)
  {
    projectKey_ = projectKey;
    return *this;
  };
  Jira &setTicketKey(std::string &ticketKey)
  {
    ticketKey_ = ticketKey;
    return *this;
  };
  Jira &setIssueType(std::string &issueType)
  {
    issue_type_ = issueType;
    return *this;
  };

private:
  std::string summary_;
  std::string description_;
  std::string projectKey_;
  std::string ticketKey_;
  Poco::Net::HTTPCredentials auth_;
  const std::string baseUrl_ = "https://yourcompany.atlassian.net/rest/api/2/issue/";

  constexpr std::pair<std::string, std::string> ticket_info_map_data[] = {
      {"X-ISSUE-TYPE", ""},
      {"X-PROJECT", ""},
      {"X-SUMMARY", ""},
      {"X-DESCRIPTION", ""}};

  constexpr std::map<std::string, std::string> ticket_info_map(ticket_info_map_data, std::end(ticket_info_map_data));

  void extractTicketInfo(const std::string &input)
  {
    std::string::size_type start = input.find("jira");
    if (start == std::string::npos)
      return;

    std::string::size_type end = start + 4;
    while (end < input.length())
    {
      start = input.find(" ", end);
      if (start == std::string::npos)
        break;

      end = input.find(" ", start + 1);
      if (end == std::string::npos)
        break;

      std::string key = input.substr(start + 1, end - start - 1);
      auto iter = ticket_info_map.find(key);
      if (iter != ticket_info_map.end())
      {
        start = end;
        end = input.find(" ", start + 1);
        if (end == std::string::npos)
          end = input.length();

        iter->second = input.substr(start + 1, end - start - 1);
      }
    }
  }

  bool createTicketInternal()
  {
    Poco::URI uri(baseUrl_);
    // TODO - need to close the session on delete?
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
    request.setContentType("application/json");

    request.setCredentials(auth_);

    std::string body = "{\"fields\": {\"project\": {\"key\": \"" + projectKey_ + "\"},\"summary\": \"" + summary_ + "\",\"description\": \"" + description_ + "\",\"issuetype\": {\"name\": \"Task\"}}}";
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
      Poco::URI uri(baseUrl_ + ticketKey_);
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
      spdlog::error("JiraConnectionError: {}", e.what());
      return false;
    }
  }

  bool isValidIssueType(const std::string &str)
  {
    static const auto it = issueTypeMap.find(str);
    return it != issueTypeMap.end();
  }
};
