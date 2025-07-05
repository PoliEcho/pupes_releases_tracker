#include "glibmm/datetime.h"
#include "glibmm/ustring.h"
#include "gtkmm/button.h"
#include "gtkmm/calendar.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/entry.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/switch.h"
#include "gtkmm/window.h"
#include "macros.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "persistance.hpp"
#include "types.hpp"
#include <gtkmm.h>
#include <iostream>

#ifdef RELEASE
#include "dynamic_src.hpp"
#endif

namespace AddItemDialog {
bool is_AddItemDialogWindow_open = false;

void handle_cancel_button(Gtk::Window *AddItemDialogWindow) {
  is_AddItemDialogWindow_open = false;
  if (AddItemDialogWindow)
    AddItemDialogWindow->set_visible(false);
}

void check_specific_time_checkbox(const Glib::RefPtr<Gtk::Builder> &Builder) {
  Gtk::CheckButton *ai_specific_time_checkbox =
      Builder->get_widget<Gtk::CheckButton>("ai_specific_time_checkbox");
  ai_specific_time_checkbox->set_active(true);
}

void submit_data(const Glib::RefPtr<Gtk::Builder> &Builder) {
  Gtk::Entry *title_entry = Builder->get_widget<Gtk::Entry>("ai_title_entry");
  Gtk::Entry *type_entry = Builder->get_widget<Gtk::Entry>("ai_type_entry");
  Gtk::Switch *notif_switch =
      Builder->get_widget<Gtk::Switch>("ai_get_notif_switch");
  Gtk::Calendar *calendar_widget =
      Builder->get_widget<Gtk::Calendar>("ai_calendar");
  Glib::DateTime release_dateTime = calendar_widget->get_date();
  release_dateTime = release_dateTime.add_full(
      0, 0, 0, -release_dateTime.get_hour(), -release_dateTime.get_minute(),
      -static_cast<double>(
          static_cast<double>(release_dateTime.get_second()) +
          (static_cast<double>(release_dateTime.get_microsecond()) /
           1000000.0)));

  Gtk::CheckButton *specific_time_checkbox =
      Builder->get_widget<Gtk::CheckButton>("ai_specific_time_checkbox");

  const bool specific_time_is_set = specific_time_checkbox->get_active();
  if (specific_time_is_set) {
    Gtk::SpinButton *hours_spin_button =
        Builder->get_widget<Gtk::SpinButton>("ai_time_hour_spin");
    Gtk::SpinButton *minutes_spin_button =
        Builder->get_widget<Gtk::SpinButton>("ai_time_minute_spin");
    release_dateTime =
        release_dateTime.add_hours(hours_spin_button->get_value())
            .add_minutes(minutes_spin_button->get_value());
  }
  MainWindow::column_view_list_store->append(RowData::create(
      title_entry->get_text(), type_entry->get_text(), release_dateTime,
      specific_time_is_set, notif_switch->get_state()));
  save_list_store_to_file();
}

constexpr void connect_signals(Glib::RefPtr<Gtk::Builder> &Builder,
                               Gtk::Window *AddItemDialogWindow) {
  CONNECT_SIGNAL(Builder, Gtk::Button, "ai_cancel_button", signal_clicked,
                 handle_cancel_button, AddItemDialogWindow)
  CONNECT_SIGNAL(Builder, Gtk::Button, "ai_ok_button", signal_clicked,
                 submit_data, Builder)

  CONNECT_SIGNAL(Builder, Gtk::SpinButton, "ai_time_hour_spin",
                 signal_value_changed, check_specific_time_checkbox, Builder)
  CONNECT_SIGNAL(Builder, Gtk::SpinButton, "ai_time_minute_spin",
                 signal_value_changed, check_specific_time_checkbox, Builder)
}

constexpr void
setup_entry_popup_menu_handlers(Glib::RefPtr<Gtk::Builder> &Builder) {
  Gtk::Entry *title_entry = Builder->get_widget<Gtk::Entry>("ai_type_entry");

  CONNECT_ACTION("select-movie", title_entry->set_text("Movie");, title_entry)

  CONNECT_ACTION("select-tvshow", title_entry->set_text("TV show");
                 , title_entry)

  CONNECT_ACTION("select-game", title_entry->set_text("Game");, title_entry)

  CONNECT_ACTION("select-book", title_entry->set_text("Book");, title_entry)
}

void window_start() {
  if (is_AddItemDialogWindow_open) {
    return;
  }
  is_AddItemDialogWindow_open = true;
  Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
  try {
#ifndef RELEASE
    Builder->add_from_file("resources/ui/add_item_window.ui");
#else
    Builder->add_from_string((char *)resources_ui_add_item_window_ui);
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

  Gtk::Window *AddItemDialogWindow =
      Builder->get_widget<Gtk::Window>("AddItemDialogWindow");
  if (!AddItemDialogWindow) {
    std::cerr << "Could not get the dialog" << std::endl;
    return;
  };
  connect_signals(Builder, AddItemDialogWindow);
  setup_entry_popup_menu_handlers(Builder);

  AddItemDialogWindow->signal_hide().connect(
      [AddItemDialogWindow]() { delete AddItemDialogWindow; });

  AddItemDialogWindow->signal_close_request().connect(
      []() -> bool {
        is_AddItemDialogWindow_open = false;
        return false;
      },
      false);

  AddItemDialogWindow->set_visible(true);
}
} // namespace AddItemDialog