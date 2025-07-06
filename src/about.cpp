
#include "dynamic_src.hpp"
#include "gdkmm/pixbuf.h"
#include "glibmm/refptr.h"
#include "gtkmm/aboutdialog.h"
#include "gtkmm/builder.h"
#include "main.hpp"
#include <fstream>
#include <glibmm/binding.h>
#include <gtkmm.h>
#include <iostream>

namespace AboutDialog {
bool about_dialog_open = false;
void show_about_window() {
  if (!about_dialog_open) {
    about_dialog_open = true;
    Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
    try {
#ifndef RELEASE
      Builder->add_from_file("resources/ui/about.ui");
#else
      Builder->add_from_string((char *)resources_ui_about_ui);
#endif
    } catch (const Glib::FileError &ex) {
      std::cerr << "FileError: " << ex.what() << std::endl;
      return;
    } catch (const Glib::MarkupError &ex) {
      std::cerr << "MarkupError: " << ex.what() << std::endl;
      return;
    } catch (const Gtk::BuilderError &ex) {
      std::cerr << "BuilderError: " << ex.what() << std::endl;
      return;
    }
    Gtk::AboutDialog *about_dialog =
        Builder->get_widget<Gtk::AboutDialog>("about_dialog");

    about_dialog->signal_close_request().connect(
        []() -> bool {
          about_dialog_open = false;
          return false;
        },
        false);
    about_dialog->set_logo([]() -> Glib::RefPtr<Gdk::Texture> {
#ifdef RELEASE
      GBytes *app_logo_bytes = g_bytes_new_static(resources_img_icon_svg,
                                                  resources_img_icon_svg_len);
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
    }());
    about_dialog->show();
  }
}
} // namespace AboutDialog