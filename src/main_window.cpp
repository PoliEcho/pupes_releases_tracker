#include "main_window.hpp"
#include "gtkmm/button.h"
#include "main.hpp"
#include <gtkmm.h>
#include <iostream>
namespace MainWindow {
Gtk::Window *MainWindow = nullptr;

void on_button_clicked() {
  if (MainWindow)
    MainWindow->set_visible(false); // will cause app to terminate
}

void on_app_activate() {
  // Load the GtkBuilder file and instantiate its widgets:
  Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
  try {
    Builder->add_from_file("resources/ui/main_window.ui");
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

  MainWindow = Builder->get_widget<Gtk::Window>("MainWindow");
  if (!MainWindow) {
    std::cerr << "Could not get the dialog" << std::endl;
    return;
  };
  Gtk::Button *add_item_Button =
      Builder->get_widget<Gtk::Button>("mw_add_item_button");
  add_item_Button->signal_clicked().connect([]() { on_button_clicked(); });

  MainWindow->signal_hide().connect([]() { delete MainWindow; });

  app->add_window(*MainWindow);
  MainWindow->set_visible(true);
}
} // namespace MainWindow
