#pragma once
#include <glibmm/object.h>
#include <glibmm/ustring.h>

class RowData : public Glib::Object {
public:
  Glib::ustring name;
  Glib::ustring type;
  Glib::ustring release_date;
  Glib::ustring releases_in;

  // Creation method
  static Glib::RefPtr<RowData> create(const Glib::ustring &name,
                                      const Glib::ustring &type,
                                      const Glib::ustring &release_date,
                                      const Glib::ustring &releases_in) {
    return Glib::make_refptr_for_instance(
        new RowData(name, type, release_date, releases_in));
  }

private:
  // Constructor
  RowData(const Glib::ustring &name, const Glib::ustring &type,
          const Glib::ustring &release_date, const Glib::ustring &releases_in)
      : name(name), type(type), release_date(release_date),
        releases_in(releases_in) {}
};
