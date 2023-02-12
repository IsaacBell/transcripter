cmake_minimum_required(VERSION 3.10)
project(test)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(/usr/src/gtest)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
# if (CMAKE_VERSION VERSION_LESS 2.8.11)
#     include_directories("/path/to/googletest/include")
# endif()

# Now simply link your own targets against gtest, gtest_main,
# etc. as appropriate
add_executable(transcripter
    ./API/Jira.hpp
    ./API/OpenAI.hpp
    ./Engine/Engine.hpp
    ./Engine/IEngine.hpp
    ./Service/Mail.hpp
    ./Service/Transcribe.hpp
    ./Test/CMakeLists.txt
    ./Workers/Client.hpp
    ./Workers/MessageBus.hpp
    ./Workers/PocoHandler.cpp
    ./Workers/PocoHandler.hpp
    ./Workers/README.md
    ./Workers/policies/SendEmail.hpp
    ./Workers/policies/Test.hpp
    ./Workers/policies/TranscribeVideo.hpp
    ./Workers/policies/google/ScheduleGoogleMeeting.hpp
    ./Workers/policies/jira/CreateJiraTicket.hpp
    main.cpp
)

target_link_libraries(transcripter gtest gtest_main)

# Google Test documentation provides more options to configure
# your test suite.
