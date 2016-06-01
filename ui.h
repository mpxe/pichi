// generated by Fast Light User Interface Designer (fluid) version 1.0302

#ifndef ui_h
#define ui_h
#include <FL/Fl.H>
namespace gnss { class Transceiver; }; 
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

class Ui {
public:
  Ui(gnss::Transceiver* t);
private:
  Fl_Double_Window *main_window;
  Fl_Light_Button *button_start;
  inline void cb_button_start_i(Fl_Light_Button*, void*);
  static void cb_button_start(Fl_Light_Button*, void*);
  Fl_Choice *choice_mode;
  static Fl_Menu_Item menu_choice_mode[];
public:
  Fl_Output *text_rate;
  Fl_Output *text_total;
private:
  Fl_Input *text_device_id;
  Fl_Input *text_gnss_port;
  Fl_Input *text_trans_ip;
  Fl_Input *text_trans_port;
  Fl_Input *text_recv_ip;
  Fl_Input *text_recv_port;
public:
  Fl_Button *button_apply;
private:
  inline void cb_button_apply_i(Fl_Button*, void*);
  static void cb_button_apply(Fl_Button*, void*);
public:
  void show(int argc, char** argv);
  static void update_status_callback(void* p);
  void update_status();
private:
  void apply_settings();
  void load_settings();
  void button_start_clicked();
  void button_apply_clicked();
  gnss::Transceiver* transceiver_; 
  unsigned long long last_count_; 
};
#endif
