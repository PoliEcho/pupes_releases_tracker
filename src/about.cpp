
#include "dynamic_src.hpp"
#include "gdkmm/pixbuf.h"
#include "glibmm/refptr.h"
#include "gtkmm/aboutdialog.h"
#include "gtkmm/builder.h"
#include "main.hpp"
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
    about_dialog->set_logo(app_icon);
    about_dialog->show();
  }
}
} // namespace AboutDialog