FROM debian
WORKDIR /app
COPY worker /app
COPY in /app
RUN apt-get update && apt-get install -y procps && rm -rf /var/lib/apt/lists/*
CMD ./worker in out
