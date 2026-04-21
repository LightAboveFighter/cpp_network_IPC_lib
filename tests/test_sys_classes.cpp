#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

#include "../sys_classes.h"

TEST(SocketTest, ConnectionAndDataExchange) {
  Socket server_socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_address {};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  server_address.sin_port = 0;

  server_socket.bind_s((sockaddr*)&server_address);
  server_socket.listen_s(5);

  socklen_t server_address_size = sizeof(server_address);
  getsockname(server_socket.get_f_descriptor(), (sockaddr*)&server_address,
              &server_address_size);
  in_port_t port = ntohs(server_address.sin_port);

  Socket client_socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_port = htons(port);
  client_socket.connect_s((sockaddr*)&server_address);

  Socket client_from_server = server_socket.accept_s();

  const char* test_message_to_server = "Test message to Server";
  size_t to_server_size = strlen(test_message_to_server) + 1;
  client_socket.writeAll((void*)test_message_to_server, to_server_size);

  char buffer[256] = {0};
  client_from_server.readAll(buffer, to_server_size);
  EXPECT_STREQ(buffer, test_message_to_server);

  const char* test_message_to_client = "Test message to Client";
  size_t to_client_size = strlen(test_message_to_client) + 1;
  client_from_server.writeAll((void*)test_message_to_client, to_client_size);

  char client_buffer[256] = {0};
  client_socket.readAll(client_buffer, to_client_size);
  EXPECT_STREQ(client_buffer, test_message_to_client);

  int poll_result = client_socket.poll_s(POLLIN, 0);
  EXPECT_EQ(poll_result, 0);
}

TEST(SocketTest, ConnectToNonExistentPortThrows) {
  Socket socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(54321);

  EXPECT_THROW(socket.connect_s((sockaddr*)&addr), std::system_error);
}

TEST(NamedPipeTest, WriteAndReadData) {
  auto tmp_path =
      std::filesystem::temp_directory_path() / "test_named_pipe_temp";
  std::string path_template = tmp_path.string();
  char* path_cstr = const_cast<char*>(path_template.c_str());
  if (mkstemp(path_cstr) != -1) {
    std::filesystem::remove(path_cstr);
  }
  std::filesystem::path named_pipe_path(path_cstr);

  struct Cleanup {
    std::filesystem::path p;
    ~Cleanup() { std::filesystem::remove(p); }
  } cleanup{named_pipe_path};

  std::thread writer_thread([&named_pipe_path]() {
    NamedPipe writer(named_pipe_path, O_WRONLY, 0666, false);
    const char* data = "Test data from NamedPipe";
    size_t data_size = strlen(data) + 1;
    writer.write_p((void*)data, data_size);
  });

  NamedPipe reader(named_pipe_path, O_RDONLY, 0666, true);
  char buffer[256] = {0};
  reader.read_p(buffer, sizeof("Test data from NamedPipe"));

  EXPECT_STREQ(buffer, "Test data from NamedPipe");

  writer_thread.join();
}

TEST(NamedPipeTest, PollWorksCorrectly) {
  auto tmp_path =
      std::filesystem::temp_directory_path() / "test_named_pipe_poll_temp";
  std::string path_template = tmp_path.string();
  char* path_cstr = const_cast<char*>(path_template.c_str());
  if (mkstemp(path_cstr) != -1) {
    std::filesystem::remove(path_cstr);
  }
  std::filesystem::path named_pipe_path(path_cstr);
  struct Cleanup {
    std::filesystem::path p;
    ~Cleanup() { std::filesystem::remove(p); }
  } cleanup{named_pipe_path};

  std::thread writer([&named_pipe_path]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    NamedPipe named_pipe(named_pipe_path, O_WRONLY, 0666, false);
    char c = 'X';
    named_pipe.write_p(&c, 1);
  });

  NamedPipe reader(named_pipe_path, O_RDONLY, 0666, true);

  int poll_result = reader.poll_s(POLLIN, 2000);
  EXPECT_EQ(poll_result, 1);

  char symbol;
  reader.read_p(&symbol, 1);
  EXPECT_EQ(symbol, 'X');

  writer.join();
}