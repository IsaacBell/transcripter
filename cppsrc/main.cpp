#include <string>

#include "Engine/Engine.hpp"

int main(int argc, _TCHAR *argv[])
{
  std::string url = "https://example.com/sample.mp4";

  Transcriptor::Engine engine(url);
  engine.start();
  engine.runTestWorker();

  return 0;
}
