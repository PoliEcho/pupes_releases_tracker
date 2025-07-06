#include "main.hpp"
#include "dynamic_src.hpp"
#include "glib.h"
#include "main_window.hpp"
#include "persistance.hpp"
#include "systray_conn.hpp"
#include <filesystem>
#include <fstream>
#include <gtkmm/builder.h>
#include <string>

Glib::RefPtr<Gtk::Application> app;

int main() {
  app = Gtk::Application::create("org.pupes.releases-tracker");

  app->signal_activate().connect([]() { MainWindow::start_main_window(); });

  {
    std::string icon_path = get_save_dirpath() + "/PRT-icon.svg";
    std::filesystem::path file_path(icon_path);
    if (!std::filesystem::exists(file_path)) {
      std::ofstream icon_file(icon_path, std::ios::binary | std::ios::out);
      if (icon_file.is_open()) {
        icon_file.write(reinterpret_cast<const char *>(resources_img_icon_svg),
                        resources_img_icon_svg_len);
        icon_file.close();
      }
    }
  }

  Systray::init_sytray();

  return app->run();
}