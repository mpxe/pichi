// Implementation of functions declared in FLUID designer

#include "mainwindow.h"


#include <FL/Fl.H>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "../pichi/configuration.h"
#include "../pichi/packet.h"
#include "../pichi/device.h"
#include "../pichi/pichi.h"


void MainWindow::init()
{
  // Must be set or callbacks will be called with nullptr
  choice_display_device->user_data(this);
  radio_recv_log_full->user_data(this);
  radio_recv_log_short->user_data(this);

  display_add_device(pichi::Device::LOCAL_DEVICE_ID);
  choice_display_device->value(0);
}


void MainWindow::apply_settings()
{
  pichi::Configuration conf{pichi_->config()};
  try {
    conf.device_id = std::stoul(text_device_id->value());

    conf.gnss_port = text_gnss_port->value();
    conf.gnss_port_rate = std::stoul(text_gnss_port_rate->value());

    conf.trans_ip = text_trans_ip->value();
    conf.trans_port = std::stoul(text_trans_port->value());

    conf.recv_ip = text_recv_ip->value();
    conf.recv_port = std::stoul(text_recv_port->value());
    conf.recv_log = check_recv_log->value();
    conf.recv_log_format = pichi::LogFormat::Short;  // Default
    if (radio_recv_log_full->value())
      conf.recv_log_format = pichi::LogFormat::Full;

    conf.log_csv = check_log_csv->value();
    conf.log_gpx = check_log_gpx->value();
    conf.log_csv_force_1hz = check_log_csv_force_1hz->value();
    conf.log_gpx_force_1hz = check_log_gpx_force_1hz->value();

    pichi_->set_config(conf);
  }
  catch (const std::invalid_argument& e) {
    std::cerr << "Error applying settings: " << e.what()
              << "\nSettings were not applied." << std::endl;
  }
  catch (const std::out_of_range& e) {
    std::cerr << "Error applying settings: " << e.what()
              << "\nSettings were not applied." << std::endl;
  }
}


void MainWindow::load_settings()
{
  auto& conf = pichi_->config();

  text_device_id->value(std::to_string(conf.device_id).c_str());

  text_gnss_port->value(conf.gnss_port.c_str());
  text_gnss_port_rate->value(std::to_string(conf.gnss_port_rate).c_str());

  text_trans_ip->value(conf.trans_ip.c_str());
  text_trans_port->value(std::to_string(conf.trans_port).c_str());

  text_recv_ip->value(conf.recv_ip.c_str());
  text_recv_port->value(std::to_string(conf.recv_port).c_str());
  check_recv_log->value(conf.recv_log);
  radio_recv_log_full->value(0);
  radio_recv_log_short->value(0);
  switch (conf.recv_log_format) {
    case pichi::LogFormat::Full: radio_recv_log_full->value(1); break;
    case pichi::LogFormat::Short: radio_recv_log_short->value(1); break;
  }

  check_log_csv->value(conf.log_csv);
  check_log_gpx->value(conf.log_gpx);
  check_log_csv_force_1hz->value(conf.log_csv_force_1hz);
  check_log_gpx_force_1hz->value(conf.log_gpx_force_1hz);
}


void MainWindow::button_start_clicked()
{
  if (button_start->value()) {
    try {
      switch (choice_mode->value()) {
        case 0: pichi_->start_transmitter(); break;
        case 1: pichi_->start_receiver(); break;
        case 2: pichi_->start_logger(); break;
        case 3: pichi_->start_device(); break;
        case 4: pichi_->start_debug_mode(); break;
      }
    }
    catch (const pichi::Error& e) {
      std::cerr << e.what() << std::endl;
    }

    button_start->label("Stop");
    last_count_ = 0;
    Fl::add_timeout(1.0, &MainWindow::update_status_callback, this);

    if (choice_mode->value() < 4) {
      Fl::add_timeout(0.5, &MainWindow::update_display_callback, this);
    }
  }
  else {
    pichi_->stop();
    button_start->label("Start");
    Fl::remove_timeout(&MainWindow::update_status_callback, this);
    Fl::remove_timeout(&MainWindow::update_display_callback, this);
  }
}


void MainWindow::button_apply_clicked()
{
  apply_settings();
}


void MainWindow::button_sync_time_clicked()
{
  // This is just for convenience and not intented as a proper time sync method
  if (!pichi_->is_active()) {
    // Rough time sync via NTP
    std::cout << "Syncing time, please wait..." << std::endl;
    std::thread{[]{system("sudo service ntp stop && sudo ntpd -gq && sudo service ntp start");}
    }.detach();
  }
  else
    std::cerr << "Can't sync time while running!" << std::endl;
}


void MainWindow::radio_log_clicked_callback(Fl_Round_Button* o, void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->radio_log_clicked(o);
}


void MainWindow::radio_log_clicked(Fl_Round_Button* o)
{
  // Disable callbacks
  radio_recv_log_full->when(0);
  radio_recv_log_short->when(0);

  radio_recv_log_full->value(0);
  radio_recv_log_short->value(0);

  o->value(1);

  radio_recv_log_full->when(FL_WHEN_CHANGED);
  radio_recv_log_short->when(FL_WHEN_CHANGED);
}


void MainWindow::update_status_callback(void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->update_status();
  Fl::repeat_timeout(1.0, &MainWindow::update_status_callback, w);
}


void MainWindow::update_status()
{
  // Rate (intented to be in hz) only works when MainWindow is updated each second
  auto count = pichi_->activity_count();
  auto rate = count - last_count_;
  last_count_ = count;

  text_rate->value(std::to_string(rate).c_str());
  text_total->value(std::to_string(count).c_str());
}


void MainWindow::update_display_callback(void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->update_display();
  Fl::repeat_timeout(1.0, &MainWindow::update_display_callback, w);
}


void MainWindow::update_display()
{
  bool success = false;
  pichi::LocationPacket location;
  std::tie(success, location) = pichi_->get_location(display_device_id_);

  if (success) {
    text_display_utc->value(std::to_string(location.utc_timestamp).c_str());
    text_display_lat->value(std::to_string(location.latitude).c_str());
    text_display_long->value(std::to_string(location.longitude).c_str());
  }

  // TODO: Fix
  //auto ids = pichi_->new_device_ids();
  //for (auto i : ids)
  //  display_add_device(i);
}


void MainWindow::display_device_changed_callback(Fl_Choice* o, void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->display_device_changed(w->mapped_device_id(o->value()));
}


void MainWindow::display_device_changed(uint16_t id)
{
  display_device_id_ = id;
}


void MainWindow::display_add_device(uint16_t id)
{
  std::string name = std::to_string(id);
  choice_display_device->add(name.c_str(), 0, nullptr);
  int index = choice_display_device->find_index(name.c_str());
  mapped_device_ids_[index] = id;
}


uint16_t MainWindow::mapped_device_id(int index)
{
  return mapped_device_ids_[index];
}
