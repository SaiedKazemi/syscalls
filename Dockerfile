FROM gcc:9.3 as builder
COPY . /src
WORKDIR /src
RUN make

FROM alpine:latest
MAINTAINER Saied Kazemi "saied@google.com"

WORKDIR /syscalls
COPY --from=builder /src/syscalls /src/do_nothing /src/get_killed /src/LARGE_FILE /syscalls/
CMD ["/syscalls"]
