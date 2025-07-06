#ifndef CONNECT_SIGNAL
#define CONNECT_SIGNAL(Builder, T, what, signal, where, ...)                   \
  {                                                                            \
    T *widget = Builder->get_widget<T>(what);                                  \
    widget->signal().connect([__VA_ARGS__]() { where(__VA_ARGS__); });         \
  }
#define CONNECT_ACTION(action, code, ...)                                      \
  {                                                                            \
    Glib::RefPtr<Gio::SimpleAction> Action =                                   \
        Gio::SimpleAction::create(action);                                     \
    Action->signal_activate().connect(                                         \
        [__VA_ARGS__](const Glib::VariantBase &) { code });                    \
    app->add_action(Action);                                                   \
  }

#define CONNECT_PROPERTY_CNAGES(row_data, label, column_internal, column,      \
                                check)                                         \
  {                                                                            \
    label->set_text(row_data->column_internal.get_value());                    \
    row_data->connect_property_changed(column, [label, row_data]() {           \
      if (check) {                                                             \
        label->set_text(row_data->column_internal.get_value());                \
      }                                                                        \
    });                                                                        \
  }
#endif