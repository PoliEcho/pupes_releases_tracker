#pragma once
#include "glib.h"
#include "glibmm/datetime.h"
#include "glibmm/main.h"
#include "sigc++/functors/mem_fun.h"
#include <array>
#include <cstdint>
#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <iostream>

class RowData : public Glib::Object {
public:
  Glib::ustring name;
  Glib::ustring type;
  Glib::DateTime release_date;
  Glib::ustring release_date_text;
  Glib::ustring releases_in;

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
    const std::array<Glib::ustring, 6> time_Strings{"years", "months", "days",
                                                    "year",  "month",  "day"};

    Glib::TimeSpan time_to_release_microseconds = release_date.difference(now);
    if (time_to_release_microseconds <= 0) {
      releases_in = "released!";
    } else {
      if (time_to_release_microseconds < 86400000000) {
        if (specific_time_is_set) {
          double hours =
              static_cast<double>(time_to_release_microseconds) / 3600000000;
          if (static_cast<uint8_t>(hours) >= 1) {
            const std::array<Glib::ustring, 2> hour_strings{"hour", "hours"};
            uint8_t offset = 0;
            if (hours > 1) {
              offset++;
            }
            releases_in = Glib::ustring::sprintf(
                "%d %s", static_cast<uint8_t>(hours), hour_strings[offset]);
          } else {
            double minutes = hours * 60;
            if (minutes < 1) {
              releases_in = "released!";
            } else {
              const std::array<Glib::ustring, 2> minute_strings{"minute",
                                                                "minutes"};
              uint8_t offset = 0;
              if (static_cast<uint8_t>(minutes) > 1) {
                offset++;
              }
              releases_in =
                  Glib::ustring::sprintf("%d %s", static_cast<uint8_t>(minutes),
                                         minute_strings[offset]);
            }
          }
        } else {
          releases_in = Glib::ustring::sprintf("1 %s", time_Strings[5]);
        }
      } else {
        // make tupple that stores reamining years, months and days
        const std::array<int, time_Strings.size() / 2> date_difference{
            release_date.get_year() - now.get_year(),
            release_date.get_month() - now.get_month(),
            release_date.get_day_of_month() - now.get_day_of_month()};

        releases_in = "";
        uint8_t gramar_offset;
        for (uint8_t i = 0; i < date_difference.size(); i++) {
          if (date_difference[i] > 0) {
            (date_difference[i] > 1) ? gramar_offset = 0
                                     : gramar_offset = date_difference.size();
            releases_in.append(
                Glib::ustring::sprintf("%d %s, ", date_difference[i],
                                       time_Strings[i + gramar_offset]));
          }
        }
        releases_in.erase(releases_in.length() - 2);
      }
    }

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
    std::clog << "arming timer\n" << releases_in << "\n";
    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &RowData::calculate_release_in), timeout);
    return false;
  }

  // Constructor
  RowData(const Glib::ustring &name, const Glib::ustring &type,
          const Glib::DateTime &release_date, bool specific_time_is_set,
          bool get_notifications)
      : name(name), type(type), release_date(release_date),
        specific_time_is_set(specific_time_is_set),
        get_notifications(get_notifications) {
    if (specific_time_is_set) {
      release_date_text = release_date.format("%e. %b %Y %H:%M");
    } else {
      release_date_text = release_date.format("%e. %b %Y");
    }
    calculate_release_in();
  }
};
