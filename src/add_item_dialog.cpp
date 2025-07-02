#include "macros.hpp"
#include "main.hpp"
#include <gtkmm.h>
#include <iostream>

#ifdef RELEASE
#include "dynamic_src.hpp"
#endif

namespace AddItemDialog {
Gtk::Window *AddItemDialogWindow;

constexpr void connect_signals(Glib::RefPtr<Gtk::Builder> &Builder) {}

void window_start() {
  Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
  try {
#ifndef RELEASE
    Builder->add_from_file("resources/ui/add_item_window.ui");
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

  AddItemDialogWindow = Builder->get_widget<Gtk::Window>("AddItemDialogWindow");
  if (!AddItemDialogWindow) {
    std::cerr << "Could not get the dialog" << std::endl;
    return;
  };
  connect_signals(Builder);

  AddItemDialogWindow->signal_hide().connect(
      []() { delete AddItemDialogWindow; });

  app->add_window(*AddItemDialogWindow);
  AddItemDialogWindow->set_visible(true);
}
} // namespace AddItemDialog