#include "main.hpp"
#include "main_window.hpp"
#include <gtkmm/builder.h>

Glib::RefPtr<Gtk::Application> app;

int main(int argc, char **argv) {
  app = Gtk::Application::create("org.gtkmm.example");

  app->signal_activate().connect([]() { MainWindow::on_app_activate(); });

  return app->run(argc, argv);
}