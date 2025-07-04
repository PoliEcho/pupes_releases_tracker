#pragma once

#include "giomm/notification.h"
#include "glib.h"
#include "glibmm/datetime.h"
#include "glibmm/main.h"
#include "glibmm/object.h"
#include "glibmm/property.h"
#include "glibmm/ustring.h"
#include "macros.hpp"
#include "main.hpp"
#include "sigc++/functors/mem_fun.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

class RowData : public Glib::Object {
public:
  Glib::Property<Glib::ustring> name;
  Glib::Property<Glib::ustring> type;
  Glib::Property<Glib::ustring> release_date_text;
  Glib::Property<Glib::ustring> releases_in;

  Glib::DateTime release_date;
  bool specific_time_is_set;
  bool get_notifications;

  static Glib::RefPtr<RowData> create(const Glib::ustring &name,
                                      const Glib::ustring &type,
                                      const Glib::DateTime &release_date,
                                      const bool specific_time_is_set,
                                      const bool get_notifications);

  static void delete_ColumnView_List_by_prt(const RowData *RowData_ptr);

private:
  bool calculate_release_in();

  RowData(const Glib::ustring &name, const Glib::ustring &type,
          const Glib::DateTime &release_date, bool specific_time_is_set,
          bool get_notifications);
};
