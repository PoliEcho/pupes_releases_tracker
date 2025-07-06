#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <gtk/gtk.h>
#include <iostream>
#include <libappindicator/app-indicator.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

std::string get_save_dirpath() {
  std::string home = std::getenv("HOME");
  if (home.empty()) {
    std::cerr << "[ERROR] " << "HOME environment variable not set.\n";
    exit(EXIT_FAILURE);
  }
  std::string savedir_path = home;
  savedir_path.append("/.local/share/pupes-release-tracker");

  if (!std::filesystem::exists(savedir_path)) {
    if (!std::filesystem::create_directories(savedir_path)) {
      std::cerr << "[ERROR] "
                << "Failed to create directory: " << savedir_path << "\n";
      exit(EXIT_FAILURE);
    }
  }

  return savedir_path;
}

static constexpr char *socket_path = "/tmp/pupes_RT_systray_socket";
void createMenu(int sockfd);
AppIndicator *indicator;
GtkWidget *menu;

int connnect_to_socket() {
  int sockfd;
  struct sockaddr_un address;
  int len;

  // Create socket
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("Error creating socket\n");
    return 0;
  }

  // Setup address structure
  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, socket_path);
  len = sizeof(address);

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&address, len) == -1) {
    printf("Error connecting to server\n");
    return 0;
  }
  return sockfd;
}
void listen_for_msg(int sockfd);
int main() {
  // Initialize GTK
  gtk_init(0, NULL);

  // Create app indicator
  indicator = app_indicator_new("org.pupes.releases-tracker",
                                (get_save_dirpath() + "/PRT-icon.svg").c_str(),
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_title(indicator, "Pupes Releases Tracker");
  int sockfd = connnect_to_socket();
  createMenu(sockfd);
  std::thread listen_thread(listen_for_msg, sockfd);
  gtk_main();
  listen_thread.join();
}

void send_msg(const int sockfd, uint8_t msg) {
  if (send(sockfd, &msg, sizeof(msg), 0) == -1) {
    printf("Error sending message\n");
    return;
  }
}

static void onShowApp([[maybe_unused]] GtkMenuItem *menuitem,
                      gpointer sockfd_void_ptr) {
  std::cout << "Show App clicked!" << std::endl;
  // avoid needing to use -fpermisive
  send_msg((size_t)sockfd_void_ptr, 0);
}

static void onAbout([[maybe_unused]] GtkMenuItem *menuitem,
                    gpointer sockfd_void_ptr) {
  std::cout << "About clicked!" << std::endl;
  send_msg((size_t)sockfd_void_ptr, 1);
}

static void onExit([[maybe_unused]] GtkMenuItem *menuitem,
                   gpointer sockfd_void_ptr) {
  std::cout << "Exit clicked!" << std::endl;
  send_msg((size_t)sockfd_void_ptr, 2);
  gtk_main_quit();
}

void listen_for_msg(int sockfd) {
  uint8_t msg;
  uint8_t data_recv;
  do {
    msg = 0;

    data_recv = recv(sockfd, &msg, sizeof(msg), 0);

    if (data_recv > 0) {

      switch (msg) {
      case 255:
        goto exit_recv_loop;
        break;
      default:
        std::cerr << "INVALID DATA ON SOCKET!!!";
      }
    }
  } while (data_recv > 0);
exit_recv_loop:
  gtk_main_quit();
}

void createMenu(int sockfd) {
  menu = gtk_menu_new();

  // Add menu items
  GtkWidget *item_show = gtk_menu_item_new_with_label("Show/Hide App");
  GtkWidget *item_about = gtk_menu_item_new_with_label("About");
  GtkWidget *separator = gtk_separator_menu_item_new();
  GtkWidget *item_exit = gtk_menu_item_new_with_label("Exit");

  // Append items to menu
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_show);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_about);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_exit);

  // Connect signals
  g_signal_connect(item_show, "activate", G_CALLBACK(onShowApp),
                   (void *)sockfd);
  g_signal_connect(item_about, "activate", G_CALLBACK(onAbout), (void *)sockfd);
  g_signal_connect(item_exit, "activate", G_CALLBACK(onExit), (void *)sockfd);

  // Show all menu items
  gtk_widget_show_all(menu);

  // Set the menu for the indicator
  app_indicator_set_menu(indicator, GTK_MENU(menu));
}
