#include "main.hpp"
#include "dynamic_src.hpp"
#include "glib.h"
#include "main_window.hpp"
#include "systray_conn.hpp"
#include <fstream>
#include <gtkmm/builder.h>

Glib::RefPtr<Gtk::Application> app;
Glib::RefPtr<Gdk::Texture> app_icon;

int main() {
  app = Gtk::Application::create("org.pupes.releases-tracker");

  app->signal_activate().connect([]() { MainWindow::start_main_window(); });

  app_icon = []() -> Glib::RefPtr<Gdk::Texture> {
#ifdef RELEASE
    GBytes *app_logo_bytes =
        g_bytes_new_static(resources_img_icon_svg, resources_img_icon_svg_len);
#else
    std::ifstream file("resources/img/icon.svg",
                       std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      return nullptr;
    }

    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate memory and read file content
    gpointer data = g_malloc(file_size);
    file.read(static_cast<char *>(data), file_size);
    file.close();
    GBytes *app_logo_bytes = g_bytes_new_take(data, file_size);
#endif
    return Gdk::Texture::create_from_bytes(Glib::wrap(app_logo_bytes));
  }();

  Systray::init_sytray();

  return app->run();
}