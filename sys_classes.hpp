#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <filesystem>
#include <string>
#include <vector>

class Socket {
  int fd = -1;

 public:
  Socket(int domain, int type, int protocol);

  Socket(int socket_descriptor);

  ~Socket();

  Socket(Socket&& other) noexcept;

  Socket& operator=(Socket&& other) noexcept;

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  int get_f_descriptor() const;

  void connect_s(const sockaddr* server_address) const;

  void bind_s(sa_family_t sin_family, in_port_t sin_port, in_addr_t sin_addr) const;

  void writeAll(void* buffer, size_t n) const;

  void readAll(void* buffer, size_t n) const;

  int read_s(void* buffer, size_t n) const;

  void listen_s(size_t n_connections) const;

  int poll_s(short events, int timeout);

  Socket accept_s() const;

  explicit operator bool() const;

  void close_s();
};

class NamedPipe {
  int fd{-1};
  std::filesystem::path path;
  bool is_owner;

 public:
  NamedPipe(const std::filesystem::path& name, int flags, mode_t mode,
            bool is_owner);
  NamedPipe(NamedPipe&& other);
  NamedPipe& operator=(NamedPipe&& other) noexcept;

  NamedPipe(const NamedPipe&) = delete;
  NamedPipe operator=(NamedPipe& pipe) = delete;

  ~NamedPipe();

  void close_s();

  void read_p(void* buffer, size_t n);

  void write_p(void* buffer, size_t n);

  int poll_s(short events, int timeout);

  explicit operator bool() const;
};