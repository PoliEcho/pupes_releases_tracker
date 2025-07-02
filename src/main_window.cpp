#include "main_window.hpp"
#include "gtkmm/button.h"
#include "main.hpp"
#include <gtkmm.h>
#include <iostream>

#ifdef RELEASE
#include "dynamic_src.hpp"
#endif

#define CONNECT_SIGNAL(Builder, T, what, signal, where)                        \
  {                                                                            \
    T *widget = Builder->get_widget<T>(what);                                  \
    widget->signal().connect([]() { where(); });                               \
  }

namespace MainWindow {
Gtk::Window *MainWindow = nullptr;

void on_button_clicked() {
  if (MainWindow)
    MainWindow->set_visible(false); // will cause app to terminate
}

constexpr void connect_signals(Glib::RefPtr<Gtk::Builder> &Builder) {
  CONNECT_SIGNAL(Builder, Gtk::Button, "mw_add_item_button", signal_clicked,
                 on_button_clicked)
}

void on_app_activate() {
  // Load the GtkBuilder file and instantiate its widgets:
  Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
  try {
#ifndef RELEASE
    Builder->add_from_file("resources/ui/main_window.ui");
#else
    Builder->add_from_string((char *)resources_ui_main_window_ui);
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

  MainWindow = Builder->get_widget<Gtk::Window>("MainWindow");
  if (!MainWindow) {
    std::cerr << "Could not get the dialog" << std::endl;
    return;
  };
  connect_signals(Builder);

  MainWindow->signal_hide().connect([]() { delete MainWindow; });

  app->add_window(*MainWindow);
  MainWindow->set_visible(true);
}
} // namespace MainWindow
