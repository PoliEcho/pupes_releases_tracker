#include "main_window.hpp"
#include "about.hpp"
#include "add_item_dialog.hpp"
#include "gtkmm/button.h"
#include "gtkmm/columnview.h"
#include "macros.hpp"
#include "main.hpp"
#include "persistance.hpp"
#include "types.hpp"
#include <glibmm/binding.h>
#include <gtkmm.h>
#include <iostream>

#ifdef RELEASE
#include "dynamic_src.hpp"
#endif

namespace MainWindow {
Gtk::Window *MainWindow = nullptr;

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
              label->set_text(row_data->name.get_value());
              row_data->connect_property_changed("name", [label, row_data]() {
                label->set_text(row_data->name.get_value());
              });
              break;
            case 1:
              label->set_text(row_data->type.get_value());
              row_data->connect_property_changed("type", [label, row_data]() {
                label->set_text(row_data->type.get_value());
              });
              break;
            case 2:
              label->set_text(row_data->release_date_text.get_value());
              row_data->connect_property_changed(
                  "release-date-text", [label, row_data]() {
                    label->set_text(row_data->release_date_text.get_value());
                  });
              break;
            case 3:
              label->set_text(row_data->releases_in.get_value());
              row_data->connect_property_changed(
                  "releases-in", [label, row_data]() {
                    label->set_text(row_data->releases_in.get_value());
                  });
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
  CONNECT_ACTION("delete-all-items", column_view_list_store->remove_all();
                 save_list_store_to_file();)
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
      column_view_list_store->remove(selected_position);
      save_list_store_to_file();
      return true;
    }
  }
  return false;
}

void on_app_activate() {
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

  load_list_store_from_file();
  Gtk::ColumnView *column_view =
      Builder->get_widget<Gtk::ColumnView>("mw_column_view");
  inicialize_column_view(column_view);

  connect_signals(Builder);
  connect_actions();

  Glib::RefPtr<Gtk::EventControllerKey> key_controller =
      Gtk::EventControllerKey::create();
  key_controller->signal_key_pressed().connect(
      sigc::bind(sigc::ptr_fun(&on_key_pressed), column_view), false);
  MainWindow->add_controller(key_controller);

  MainWindow->signal_hide().connect([]() { delete MainWindow; });
  app->add_window(*MainWindow);
  // NO Icon since they removed option to set it from texture in GTK4
  // so i will use some standard icon
  MainWindow->set_visible(true);
}
} // namespace MainWindow
