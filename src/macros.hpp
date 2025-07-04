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

#endif