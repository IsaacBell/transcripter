#include <string>

#include "Transcribe.hpp"

int main(int argc, _TCHAR *argv[])
{
  std::string url = "https://example.com/sample.mp4";
  Transcribe(url);

  return 0;
}
