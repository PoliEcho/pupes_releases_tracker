#include "types.hpp"
#include "giomm/notification.h"
#include "glibmm/main.h"
#include "glibmm/refptr.h"
#include "macros.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include <cmath>
#include <giomm.h>
#include <giomm/dbusinterface.h>
#include <giomm/dbusproxy.h>
#include <glibmm/variant.h>
#include <unistd.h>

Glib::RefPtr<RowData> RowData::create(const Glib::ustring &name,
                                      const Glib::ustring &type,
                                      const Glib::DateTime &release_date,
                                      const bool specific_time_is_set,
                                      const bool get_notifications) {
  return Glib::make_refptr_for_instance(new RowData(
      name, type, release_date, specific_time_is_set, get_notifications));
}

void RowData::delete_ColumnView_List_by_prt(const RowData *RowData_ptr) {
  Glib::RefPtr<const RowData> RowData_ref_ptr =
      Glib::make_refptr_for_instance(RowData_ptr);
  std::pair<bool, guint> found =
      MainWindow::column_view_list_store->find(RowData_ref_ptr);
  if (found.first) {
    MainWindow::column_view_list_store->remove(found.second);
  }
}

bool RowData::calculate_release_in() {
  Glib::DateTime now = Glib::DateTime::create_now_local();
  const std::array<const char *, 6> time_Strings{"years", "months", "days",
                                                 "year",  "month",  "day"};
  Glib::TimeSpan time_to_release_microseconds = release_date.difference(now);
  Glib::ustring new_releases_in;
  bool arm_timer = true;

  if (time_to_release_microseconds <= 0) {
    new_releases_in = "released!";
    arm_timer = false;
  } else {
    if (time_to_release_microseconds < 86400000000) {
      if (specific_time_is_set) {
        double hours =
            static_cast<double>(time_to_release_microseconds) / 3600000000;

        if (hours > static_cast<double>(static_cast<double>(59) /
                                        static_cast<double>(60))) {
          uint8_t hours_rounded = std::round(hours);
          const std::array<const char *, 2> hour_strings{"hour", "hours"};
          uint8_t offset = 0;
          if (hours > 1) {
            offset++;
          }
          new_releases_in = Glib::ustring::sprintf("%d %s", hours_rounded,
                                                   hour_strings[offset]);
        } else {
          double minutes = hours * 60;
          if ((std::round(minutes) + 1) < 1) {
            new_releases_in = "released!";
            arm_timer = false;
          } else {
            uint8_t minutes_rounded = std::ceil(minutes);
            const std::array<const char *, 2> minute_strings{"minute",
                                                             "minutes"};
            uint8_t offset = 0;
            if (minutes_rounded > 1) {
              offset++;
            }
            new_releases_in = Glib::ustring::sprintf("%d %s", minutes_rounded,
                                                     minute_strings[offset]);
          }
        }
      } else {
        new_releases_in = Glib::ustring::sprintf("1 %s", time_Strings[5]);
      }
    } else {
      const std::array<int, 3> date_difference{
          release_date.get_year() - now.get_year(),
          release_date.get_month() - now.get_month(),
          release_date.get_day_of_month() - now.get_day_of_month()};
      new_releases_in = "";
      uint8_t gramar_offset;
      for (uint8_t i = 0; i < date_difference.size(); i++) {
        if (date_difference[i] > 0) {
          (date_difference[i] > 1) ? gramar_offset = 0
                                   : gramar_offset = date_difference.size();
          new_releases_in.append(Glib::ustring::sprintf(
              "%d %s, ", date_difference[i], time_Strings[i + gramar_offset]));
        }
      }
      new_releases_in.erase(new_releases_in.length() - 2);
    }
  }

  releases_in.set_value(new_releases_in);
  unsigned int timeout;
  if (time_to_release_microseconds < 120000000) {
    timeout = 1000;
  } else if (time_to_release_microseconds < 7200000000) {
    timeout = 30000;
  } else if (time_to_release_microseconds < 172800000000) {
    timeout = 1800000;
  } else {
    timeout = 43200000;
  }

  if (arm_timer) {
    timer = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &RowData::calculate_release_in), timeout);
  } else {
    if (get_notifications) {
      Glib::RefPtr<Gio::Notification> notification = Gio::Notification::create(
          Glib::ustring::sprintf("%s, just released", name.get_value()));
      notification->set_body("go check it out");
      Glib::ustring notif_button_action_name =
          Glib::ustring::sprintf("%s-button-delete-released", name.get_value());
      notification->add_button(
          "Delete this release",
          Glib::ustring::sprintf("app.%s", notif_button_action_name));
      CONNECT_ACTION(
          notif_button_action_name, delete_ColumnView_List_by_prt(this);, this)
      app->send_notification(
          Glib::ustring::sprintf("%s-released", name.get_value()),
          notification);
    }
  }
  return false;
}

RowData::RowData(const Glib::ustring &name, const Glib::ustring &type,
                 const Glib::DateTime &release_date, bool specific_time_is_set,
                 bool get_notifications)
    : Glib::ObjectBase("RowData"), name(*this, "name", name),
      type(*this, "type", type), release_date_text(*this, "release-date-text"),
      releases_in(*this, "releases-in"), release_date(release_date),
      specific_time_is_set(specific_time_is_set),
      get_notifications(get_notifications) {
  Glib::ustring date_text;
  if (specific_time_is_set) {
    date_text = release_date.format("%e. %b %Y %H:%M");
  } else {
    date_text = release_date.format("%e. %b %Y");
  }
  release_date_text.set_value(date_text);
  calculate_release_in();
}

void RowData::disarm_timer() { timer.disconnect(); }