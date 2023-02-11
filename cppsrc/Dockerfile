FROM ubuntu:20.04

WORKDIR /app

# install required packages
RUN apt-get update && apt-get install -y \
  clang \
  libpoco-dev \
  libspdlog-dev \
  libjsoncpp-dev \
  cmake \
  g++ \
  git

# clone the required dependencies
RUN git clone https://github.com/open-source-parsers/jsoncpp.git \
  && git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git

# build jsoncpp
WORKDIR /app/jsoncpp
RUN cmake . \
  && make \
  && make install

# build AMQP-CPP
WORKDIR /app/AMQP-CPP
RUN cmake . \
  && make \
  && make install

# copy the source code for the Transcripter app
COPY . .

# build the Transcripter app
RUN clang++ -std=c++20 main.cpp -o transcripter -lpoco -ljsoncpp -lamqpcpp

# set the environment variables
ENV OPENAI_API_KEY=
ENV OPENAI_ORGANIZATION_ID=
ENV AWS_TRANSCRIBE_AUTH_KEY=
ENV AWS_TRANSCRIBE_AUTH_SECRET=
ENV SMTP_SERVER_HOST=
ENV SMTP_SERVER_PORT=

CMD ["./transcripter"]
