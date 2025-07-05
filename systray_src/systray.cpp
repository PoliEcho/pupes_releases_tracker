#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gtk/gtk.h>
#include <iostream>
#include <libappindicator/app-indicator.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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

int main() {
  // Initialize GTK
  gtk_init(0, NULL);

  // Create app indicator
  indicator =
      app_indicator_new("org.pupes.releases-tracker", "applications-system",
                        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_title(indicator, "Pupes Releases Tracker");

  createMenu(connnect_to_socket());
  gtk_main();
}

void send_msg(const int sockfd, uint8_t msg) {
  if (send(sockfd, &msg, sizeof(msg), 0) == -1) {
    printf("Error sending message\n");
    return;
  }
}

static void onShowApp(GtkMenuItem *menuitem, gpointer sockfd_void_ptr) {
  std::cout << "Show App clicked!" << std::endl;
  // avoid needing to use -fpermisive
  send_msg((size_t)sockfd_void_ptr, 0);
}

static void onAbout(GtkMenuItem *menuitem, gpointer sockfd_void_ptr) {
  std::cout << "About clicked!" << std::endl;
  send_msg((size_t)sockfd_void_ptr, 1);
}

static void onExit(GtkMenuItem *menuitem, gpointer sockfd_void_ptr) {
  std::cout << "Exit clicked!" << std::endl;
  send_msg((size_t)sockfd_void_ptr, 2);
  gtk_main_quit();
}

void createMenu(int sockfd) {
  menu = gtk_menu_new();

  // Add menu items
  GtkWidget *item_show = gtk_menu_item_new_with_label("Show App");
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
