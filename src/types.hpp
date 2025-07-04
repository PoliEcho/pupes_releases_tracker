#pragma once

#include "giomm/notification.h"
#include "glibmm/datetime.h"
#include "glibmm/main.h"
#include "glibmm/object.h"
#include "glibmm/property.h"
#include "glibmm/ustring.h"
#include "main.hpp"
#include "sigc++/functors/mem_fun.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

class RowData : public Glib::Object {
public:
  // Convert displayed fields to properties for automatic updates
  Glib::Property<Glib::ustring> name;
  Glib::Property<Glib::ustring> type;
  Glib::Property<Glib::ustring> release_date_text;
  Glib::Property<Glib::ustring> releases_in;

  // Keep these as regular members since they're not displayed
  Glib::DateTime release_date;
  bool specific_time_is_set;
  bool get_notifications;

public:
  // Creation method
  static Glib::RefPtr<RowData> create(const Glib::ustring &name,
                                      const Glib::ustring &type,
                                      const Glib::DateTime &release_date,
                                      const bool specific_time_is_set,
                                      const bool get_notifications) {
    return Glib::make_refptr_for_instance(new RowData(
        name, type, release_date, specific_time_is_set, get_notifications));
  }

private:
  bool calculate_release_in() {
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
          uint8_t hours_rounded = std::round(hours);
          if (hours_rounded >= 1) {
            const std::array<const char *, 2> hour_strings{"hour", "hours"};
            uint8_t offset = 0;
            if (hours > 1) {
              offset++;
            }
            new_releases_in = Glib::ustring::sprintf("%d %s", hours_rounded,
                                                     hour_strings[offset]);
          } else {
            double minutes = hours * 60;
            if (minutes < 1) {
              new_releases_in = "released!";
              arm_timer = false;
            } else {
              uint8_t minutes_rounded = std::round(minutes);
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
        // make array that stores reamining years, months and days
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
            new_releases_in.append(
                Glib::ustring::sprintf("%d %s, ", date_difference[i],
                                       time_Strings[i + gramar_offset]));
          }
        }
        new_releases_in.erase(new_releases_in.length() - 2);
      }
    }

    // Use property setter to trigger automatic view updates
    releases_in.set_value(new_releases_in);

    unsigned int timeout;
    // 2 minutes, hours, days
    if (time_to_release_microseconds < 120000000) {
      timeout = 1000;
    } else if (time_to_release_microseconds < 7200000000) {
      timeout = 60000;
    } else if (time_to_release_microseconds < 172800000000) {
      timeout = 3600000;
    } else {
      timeout = 86400000;
    }

    if (arm_timer) {
      std::clog << "arming timer\n" << releases_in.get_value() << "\n";
      Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &RowData::calculate_release_in), timeout);
    } else {
      if (get_notifications) {
        Glib::RefPtr<Gio::Notification> notification =
            Gio::Notification::create(
                Glib::ustring::sprintf("%s, just released", name.get_value()));
        notification->set_body("go check it out");
        app->send_notification(
            Glib::ustring::sprintf("%s-released", name.get_value()),
            notification);
      }
    }
    return false;
  }

  // Constructor
  RowData(const Glib::ustring &name, const Glib::ustring &type,
          const Glib::DateTime &release_date, bool specific_time_is_set,
          bool get_notifications)
      : Glib::ObjectBase("RowData"), name(*this, "name", name),
        type(*this, "type", type),
        release_date_text(*this, "release-date-text"),
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
};
