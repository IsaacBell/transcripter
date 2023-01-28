#include <cstdlib> // Necessary for cpr library
#include <string>
#include <cpr/cpr.h>
#include "json/json.h"

// Call AWS Transcribe API to record transcripts from given mp4
std::string transcribeVideo(std::string url)
{

  cpr::Authentication auth(getenv("AWS_TRANSCRIBE_AUTH_KEY"), getenv("AWS_TRANSCRIBE_AUTH_SECRET"));
  std::string endpoint = "https://aws.endpoint.com/transcribe";

  auto r = cpr::Post(cpr::Url{endpoint}, auth,
                     cpr::Body{"{\"videoUrl\": \"" + url + "\"}"});

  return r.text;
}

// Transform string into input for OpenAI
std::string generateModifiedString(std::string_view &transcription)
{
  std::string modifiedString;
  modifiedString.append(
      "IGNORE ALL PREVIOUS INSTRUCTIONS. DO NOT start your response with a summary or introduction. Follow exactly the format I provide below. ");
  modifiedString.append(
      "Generate summaries, guidelines, deliverables, TODOS, followups, etc., categorized by speaker / team member:\n\n");
  modifiedString.append(
      "Output them STRICTLY in the following JSON format, leaving an empty array to represent fields which are left blank:\n\n");
  modifiedString.append("{\n
      \"  summaries\":[\"example summary\"],
      \"  guidelines\":[\"Example Guideline 1\", \"Example Guideline 2\"],
      \"  deliverables\":[\"Example Deliverable 1\"],
      \"  todos\":[\"Example Todo 1\"],
      \"  speakers\":[\"John Doe<john.doe@example.com>\", \"Jane Doe<jane.doe@example.com>\"],
      \"  followups\":[\"email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`\"],\n}");

  modifiedString.append(transcription);
  return modifiedString;
}

// Send the transformed input to OpenAI
std::string_view callOpenAiApi(std::string_view &transcription)
{
  const std::string api_key{std::getenv("OPENAI_API_KEY")};
  const std::string organization_id{std::getenv("OPENAI_ORGANIZATION_ID")};

  auto r = cpr::Get(
      cpr::Url{("https://api.openai.com/v1/engines/davinci/completions?text=" + transcription)},
      cpr::Header{{"Authorization", "Bearer " + api_key},
                  {"OpenAI-Client-Organization", organization_id}});

  return r.text;
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
