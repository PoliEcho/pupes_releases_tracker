#include "about.hpp"
#include "dynamic_src.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

static constexpr char *socket_path = "/tmp/pupes_RT_systray_socket";
static constexpr unsigned int nIncomingConnections = 1;

namespace Systray {
int server_sock;
int client_sock;
int create_socket() {
  server_sock = 0;
  struct sockaddr_un local;
  int len = 0;

  // Create socket
  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_sock == -1) {
    std::cerr << "Error creating socket\n";
    return 0;
  }

  // Setup address structure
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socket_path);
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);

  // Bind socket to path
  if (bind(server_sock, (struct sockaddr *)&local, len) != 0) {
    std::cerr << "Error binding socket\n";
    return 0;
  }
  return server_sock;
}

void send_msg(uint8_t msg) {
  if (send(client_sock, &msg, sizeof(msg), 0) == -1) {
    printf("Error sending message\n");
    return;
  }
}

void listen_on_socket() {
  struct sockaddr_un remote;

  if (listen(server_sock, nIncomingConnections) != 0) {
    std::cerr << "Error on listen call\n";
    return;
  }

  while (true) {
    unsigned int sock_len = 0;

    if ((client_sock = accept(server_sock, (struct sockaddr *)&remote,
                              &sock_len)) == -1) {
      std::cerr << "Error on accept() call\n";
      continue;
    }

    std::clog << "systray connected\n";

    uint8_t msg;
    uint8_t data_recv;

    do {
      msg = 255;

      data_recv = recv(client_sock, &msg, sizeof(msg), 0);

      if (data_recv > 0) {

        switch (msg) {
        case 0:
          g_main_context_invoke(
              nullptr,
              [](gpointer) -> gboolean {
                MainWindow::toggle_visibility();
                return G_SOURCE_REMOVE;
              },
              nullptr);
          break;
        case 1:
          // prevent crash from trying to modify UI from difernet thread
          g_main_context_invoke(
              nullptr,
              [](gpointer) -> gboolean {
                AboutDialog::show_about_window();
                return G_SOURCE_REMOVE;
              },
              nullptr);
          break;
        case 2:
          app->quit();
        case 255:
          goto exit_recv_loop;
          break;
        default:
          std::cerr << "INVALID DATA ON SOCKET!!!";
        }
      }
    } while (data_recv > 0);
  exit_recv_loop:

    close(client_sock);
  }

  close(server_sock);
}

void init_sytray() {
  server_sock = create_socket();
  if (server_sock == 0) {
    std::cerr << "Failed to create Socket\n";
    exit(ENOTCONN);
  }
  pid_t pid = fork();
  if (pid == 0) {
    int fd = memfd_create("", MFD_CLOEXEC);

    write(fd, build_bin_pupes_releases_tracker_systray,
          build_bin_pupes_releases_tracker_systray_len);

    // Create path to the memory file
    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);

    char *argv[] = {"pupes_releases_tracker_systray", NULL};

    // Execute the ELF binary
    execve(fd_path, argv, environ);
  }
  std::thread sock_listen_thread(listen_on_socket);
  sock_listen_thread.detach();
}
} // namespace Systray