#ifndef MW_PRT_HG__
#define MW_PRT_HG__
#include "giomm/liststore.h"
#include "glibmm/refptr.h"
#include "types.hpp"
namespace MainWindow {
extern Glib::RefPtr<Gio::ListStore<RowData>> column_view_list_store;
bool toggle_visibility();
void start_main_window();
} // namespace MainWindow
#endif