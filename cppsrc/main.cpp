#include <cstdlib> // Necessary for cpr library
#include <string>
#include <cpr/cpr.h>
#include "json/json.h"

#include "mail.hpp"
#include "interface/openai.hpp"

void executeResults(Json::Value &json)
{
  // todo - use the Task Queue for this
  for (const auto followup : json["followups"])
    switch (followup.substr(0, s.find(' '))) // 1st word in the string
    {
    case "email":
      // Example input:
      //   email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`
      Mail.send(followup);
      break;
    case "jira":
      // todo - get the actual values from the string
      // Example Input:
      //   jira X-PROJECT QNET-7 X-SUMMARY Do some dry-wall work. X-DESCRIPTION There are BATS in the walls!
      Jira()
          .setSummary("Summary")
          .setDescription("Description")
          .setProjectKey("ProjectKey")
          .setIssueType("Epic")
          .createTicket();
    default:
      break;
    }
}

// Call AWS Transcribe API to record transcripts from given mp4
std::string transcribeVideo(std::string &url)
{
  Poco::Net::HTTPCredentials auth(std::getenv("AWS_TRANSCRIBE_AUTH_KEY"), std::getenv("AWS_TRANSCRIBE_AUTH_SECRET"));

  const std::string endpoint = "https://aws.endpoint.com/transcribe";
  Poco::URI uri(endpoint + url);

  Poco::Net::HTTPClientSession session(
      uri.getHost(),
      uri.getPort());
  Poco::Net::HTTPRequest request(
      Poco::Net::HTTPRequest::HTTP_POST,
      uri.getPathAndQuery(),
      Poco::Net::HTTPMessage::HTTP_1_1);

  request.setContentType("application/json");
  request.setCredentials(auth);

  std::ostream &request_stream = session.sendRequest(request);
  request_stream << "{\"video_url\": \"" + url + "\"}";
  Poco::Net::HTTPResponse response;
  std::istream &response_stream = session.receiveResponse(response);

  std::string response_text;
  Poco::StreamCopier::copyToString(response_stream, response_text);

  return response_text;
}

// Transform string into input for OpenAI
std::string generateModifiedString(std::string_view &transcription)
{
  std::string modifiedString;
  modifiedString.append(
      "IGNORE ALL PREVIOUS INSTRUCTIONS. DO NOT start your response with a summary or introduction. Follow exactly the format I provide below. ");
  modifiedString.append(
      "I will provide captions from a meeting. Generate summaries, guidelines, deliverables, TODOS, followups, etc., categorized by speaker / team member:\n\n");
  modifiedString.append(
      "Your response should be consumable by the JsonCpp lib. Use this exact JSON format, leaving an empty array to represent fields which are left blank:\n\n");
  modifiedString.append("{\n
      \"  summaries\":[\"example summary\"],
      \"  guidelines\":[\"Example Guideline 1\", \"Example Guideline 2\"],
      \"  deliverables\":[\"Example Deliverable 1\"],
      \"  todos\":[\"Example Todo 1\"],
      \"  speakers\":[\"John Doe<john.doe@example.com>\", \"Jane Doe<jane.doe@example.com>\"],
      \"  followups\":[\"email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`\", \"jira X-PROJECT QNET-7 X-SUMMARY Do some dry-wall work. X-DESCRIPTION There are BATS in the walls!\"],\n}\n\n");

  modifiedString.append(transcription);
  return modifiedString;
}

// Send the transformed input to OpenAI
std::string_view callOpenAiApi(std::string_view &transcription)
{
  return OpenAI.get(transcription);
}

std::string_view run(std::string_view &url)
{
  if (!url)
    return "";
  return callOpenAiApi(generateModifiedString(transcribeVideo(url)));
}

int main(int argc, _TCHAR *argv[])
{
  Json::Reader reader;
  Json::Value root;

  string url = "https://example.com/sample.mp4";
  auto json = run(url);

  bool parseSuccess = reader.parse(json, root, false);
  if (parseSuccess)
  {
    // Json::StyledWriter styledWriter;
    Json::FastWriter fastWriter;

    Json::Value result = root["result"];
    result["url"] = url;

    // cout << styledWriter.write(newValue) << "\n";
    std::cout << fastWriter.write(newValue) << "\n";
  }

  return 0;
}
