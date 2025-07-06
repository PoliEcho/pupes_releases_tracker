#include "glibmm/datetime.h"
#include "glibmm/refptr.h"
#include "glibmm/ustring.h"
#include "main_window.hpp"
#include "types.hpp"
#include <filesystem>
#include <fstream>
#include <glibmm/fileutils.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

using nlohmann::json;

std::string get_save_dirpath() {
  std::string home = std::getenv("HOME");
  if (home.empty()) {
    std::cerr << "[ERROR] " << "HOME environment variable not set.\n";
    exit(EXIT_FAILURE);
  }
  std::string savedir_path = home;
  savedir_path.append("/.local/share/pupes-release-tracker");

  if (!std::filesystem::exists(savedir_path)) {
    if (!std::filesystem::create_directories(savedir_path)) {
      std::cerr << "[ERROR] "
                << "Failed to create directory: " << savedir_path << "\n";
      exit(EXIT_FAILURE);
    }
  }

  return savedir_path;
}

void save_persistent_to_file() {
  json column_view_list_store_json = {
      {"rows", json::array()},
      {"minimize to systray", MainWindow::minimize_to_systray}};

  for (guint i = 0; MainWindow::column_view_list_store->get_n_items() > i;
       i++) {
    Glib::RefPtr<RowData> current_RowData_ptr =
        MainWindow::column_view_list_store->get_item(i);
    column_view_list_store_json["rows"].push_back(
        {{"name", current_RowData_ptr->name.get_value()},
         {"type", current_RowData_ptr->type.get_value()},
         {"Release Date", current_RowData_ptr->release_date.format_iso8601()},
         {"specific time set", current_RowData_ptr->specific_time_is_set},
         {"get notifications", current_RowData_ptr->get_notifications}});
  }
  std::ofstream save_file(get_save_dirpath() + "/data");
  save_file << column_view_list_store_json;
  save_file.close();
}

void load_persistent_file() {
  std::ifstream save_file(get_save_dirpath() + "/data");
  if (save_file.is_open()) {
    try {
      MainWindow::column_view_list_store->remove_all();
      json data_file = json::parse(save_file);
      MainWindow::minimize_to_systray = data_file["minimize to systray"];
      for (json json_obj : data_file["rows"]) {
        MainWindow::column_view_list_store->append(
            RowData::create(json_obj["name"].get<std::string>(),
                            json_obj["type"].get<std::string>(),
                            Glib::DateTime::create_from_iso8601(
                                json_obj["Release Date"].get<std::string>()),
                            json_obj["specific time set"].get<bool>(),
                            json_obj["get notifications"].get<bool>()));
      }
    } catch (...) {
      std::cerr << "[ERROR]" << " loading data from file failed\n";
    }
  }
}
