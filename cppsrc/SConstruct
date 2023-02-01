import os
import subprocess

env = Environment(CXX='clang++',
                  CXXFLAGS='-std=c++20 -stdlib=libc++',
                  LINKFLAGS='-stdlib=libc++')

# Set the C++ flags
env.Append(CCFLAGS=["-Wall"])

# Get the environment variables
openai_api_key = os.environ.get("OPENAI_API_KEY")
openai_organization_id = os.environ.get("OPENAI_ORGANIZATION_ID")
aws_transcribe_auth_key = os.environ.get("AWS_TRANSCRIBE_AUTH_KEY")
aws_transcribe_auth_secret = os.environ.get("AWS_TRANSCRIBE_AUTH_SECRET")
smtp_server_host = os.environ.get("SMTP_SERVER_HOST")
smtp_server_port = os.environ.get("SMTP_SERVER_PORT")

# Check if the environment variables are set
if not openai_api_key:
    print("OPENAI_API_KEY is not set")
if not openai_organization_id:
    print("OPENAI_ORGANIZATION_ID is not set")
if not aws_transcribe_auth_key:
    print("AWS_TRANSCRIBE_AUTH_KEY is not set")
if not aws_transcribe_auth_secret:
    print("AWS_TRANSCRIBE_AUTH_SECRET is not set")
if not smtp_server_host:
    print("SMTP_SERVER_HOST is not set")
if not smtp_server_port:
    print("SMTP_SERVER_PORT is not set")


# Add the environment variables
os.environ["OPENAI_API_KEY"] = openai_api_key
os.environ["OPENAI_ORGANIZATION_ID"] = openai_organization_id
os.environ["AWS_TRANSCRIBE_AUTH_KEY"] = aws_transcribe_auth_key
os.environ["AWS_TRANSCRIBE_AUTH_SECRET"] = aws_transcribe_auth_secret
os.environ["SMTP_SERVER_HOST"] = smtp_server_host
os.environ["SMTP_SERVER_PORT"] = smtp_server_port

# Build the app
target = env.Program("Transcriber")

# Run the tests

# def run_tests(target, source, env):
#     subprocess.check_call(["./Transcriber"])

# test = env.Action(run_tests, None)
# AlwaysBuild(test)

# Depends(test, target) # Depend the tests on the target
