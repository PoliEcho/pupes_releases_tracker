#ifndef CONNECT_SIGNAL
#define CONNECT_SIGNAL(Builder, T, what, signal, where)                        \
  {                                                                            \
    T *widget = Builder->get_widget<T>(what);                                  \
    widget->signal().connect([]() { where(); });                               \
  }
#endif