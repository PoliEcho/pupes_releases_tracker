#include "main.hpp"
#include "main_window.hpp"
#include <gtkmm/builder.h>

Glib::RefPtr<Gtk::Application> app;

int main() {
  app = Gtk::Application::create("org.pupes.releases-tracker");

  app->signal_activate().connect([]() { MainWindow::on_app_activate(); });

  return app->run();
}