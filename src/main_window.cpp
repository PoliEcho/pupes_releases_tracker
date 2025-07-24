#include "main_window.hpp"
#include "about.hpp"
#include "add_item_dialog.hpp"
#include "glib.h"
#include "gtkmm/button.h"
#include "gtkmm/columnview.h"
#include "macros.hpp"
#include "main.hpp"
#include "persistance.hpp"
#include "systray_conn.hpp"
#include "types.hpp"
#include <cstddef>
#include <glibmm/binding.h>
#include <gtkmm.h>
#include <gtkmm/settings.h>
#include <iostream>

#ifdef RELEASE
#include "dynamic_src.hpp"
#endif

namespace MainWindow {
Gtk::Window *MainWindow = nullptr;
bool window_visible = true;
bool minimize_to_systray = true;

Glib::RefPtr<Gio::ListStore<RowData>> column_view_list_store =
    Gio::ListStore<RowData>::create();

void inicialize_column_view(Gtk::ColumnView *column_view) {
  Glib::RefPtr<Gtk::SingleSelection> selection =
      Gtk::SingleSelection::create(column_view_list_store);

  column_view->set_model(selection);

  Glib::RefPtr<Gio::ListModel> columns = column_view->get_columns();

  for (size_t i = 0; i < columns->get_n_items(); ++i) {
    Glib::RefPtr<Gtk::ColumnViewColumn> column =
        std::dynamic_pointer_cast<Gtk::ColumnViewColumn>(
            columns->get_object(i));
    Glib::RefPtr<Gtk::SignalListItemFactory> factory =
        Gtk::SignalListItemFactory::create();

    factory->signal_setup().connect(
        [](const Glib::RefPtr<Gtk::ListItem> &item) {
          item->set_child(*Gtk::make_managed<Gtk::Label>());
        });

    factory->signal_bind().connect(
        [i](const Glib::RefPtr<Gtk::ListItem> &item) {
          Gtk::Label *label = dynamic_cast<Gtk::Label *>(item->get_child());
          Glib::RefPtr<RowData> row_data =
              std::dynamic_pointer_cast<RowData>(item->get_item());
          if (label && row_data) {

            switch (i) {
            case 0:
              CONNECT_PROPERTY_CNAGES(row_data, label, name, "name",
                                      window_visible)
              break;
            case 1:
              CONNECT_PROPERTY_CNAGES(row_data, label, type, "type",
                                      window_visible)
              break;
            case 2:
              CONNECT_PROPERTY_CNAGES(row_data, label, release_date_text,
                                      "release-date-text", window_visible)
              break;
            case 3:
              CONNECT_PROPERTY_CNAGES(row_data, label, releases_in,
                                      "releases-in", window_visible)
              break;
            default:
              label->set_text("");
            }
          }
        });

    column->set_factory(factory);
  }
}

constexpr void connect_signals(Glib::RefPtr<Gtk::Builder> &Builder) {
  CONNECT_SIGNAL(Builder, Gtk::Button, "mw_add_item_button", signal_clicked,
                 AddItemDialog::window_start)
}
constexpr void connect_actions() {
  CONNECT_ACTION("show-about", AboutDialog::show_about_window();)
  CONNECT_ACTION("new-item", AddItemDialog::window_start();)
  CONNECT_ACTION(
      "delete-all-items",
      for (guint i = 0; column_view_list_store->get_n_items() > i; i++) {
        column_view_list_store->get_item(i)->disarm_timer();
      } column_view_list_store->remove_all();
      save_persistent_to_file();)

  {
    Glib::RefPtr<Gio::SimpleAction> action = Gio::SimpleAction::create_bool(
        "minimize-to-systray-toggle", minimize_to_systray);
    app->add_action(action);
    action->signal_activate().connect(
        [action]([[maybe_unused]] const Glib::VariantBase &parameter) {
          minimize_to_systray =
              !action->get_state_variant().get_dynamic<bool>();
          action->set_state(Glib::Variant<bool>::create(minimize_to_systray));
          save_persistent_to_file();
        });
  }
}

bool on_key_pressed(guint keyval, [[maybe_unused]] guint keycode,
                    [[maybe_unused]] Gdk::ModifierType state,
                    Gtk::ColumnView *column_view) {
  if (keyval == GDK_KEY_Delete) {
    Glib::RefPtr<Gtk::SingleSelection> selection =
        std::dynamic_pointer_cast<Gtk::SingleSelection>(
            column_view->get_model());

    if (selection && selection->get_selected_item()) {
      guint selected_position = selection->get_selected();
      column_view_list_store->get_item(selected_position)->disarm_timer();
      column_view_list_store->remove(selected_position);
      save_persistent_to_file();
      return true;
    }
  }
  return false;
}

bool toggle_visibility() {
  if (window_visible) {
    app->hold();
    MainWindow->hide();
    window_visible = false;
  } else {
    start_main_window();
    app->release();
    window_visible = true;
  }

  return false;
}

void start_main_window() {
  Glib::RefPtr<Gtk::Builder> Builder = Gtk::Builder::create();
  connect_actions();
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

  load_persistent_file();
  Gtk::ColumnView *column_view =
      Builder->get_widget<Gtk::ColumnView>("mw_column_view");
  inicialize_column_view(column_view);

  connect_signals(Builder);

  Glib::RefPtr<Gtk::EventControllerKey> key_controller =
      Gtk::EventControllerKey::create();
  key_controller->signal_key_pressed().connect(
      sigc::bind(sigc::ptr_fun(&on_key_pressed), column_view), false);
  MainWindow->add_controller(key_controller);

  MainWindow->signal_hide().connect([]() { delete MainWindow; });

  MainWindow->signal_close_request().connect(
      []() -> bool {
        if (minimize_to_systray) {
          toggle_visibility();
          return true;
        } else {
          Systray::send_msg(255);
          app->quit();
          return false;
        }
      },
      false);

  app->add_window(*MainWindow);

  // NO Icon since they removed option to set it from texture in GTK4
  // so i will use some standard icon
  MainWindow->set_visible(true);
}
} // namespace MainWindow
