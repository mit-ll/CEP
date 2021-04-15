#include <signal.h> 
#include <stdio.h> 
#include "vpi_user.h" 
 
int gotusr2 = 0; 
void dosig2(int d) 
{ 
  vpi_printf("!!!! Got signusr2 !!!!\n"); 
  gotusr2=1; 
}

int watcher(p_cb_data cb_data_p) 
{ 
  s_cb_data next_cb = {cbAfterDelay, watcher, 0, 0, 0, 0 }; 
  s_vpi_time t = {vpiScaledRealTime}; 
  vpiHandle top, itr; 
 
  itr= vpi_iterate(vpiModule,0); 
  top = vpi_scan(itr); 
  vpi_get_time(top,&t); 


  if(gotusr2) 
  { 
    vpi_printf("Got USR2 signal at %1.2f\n", t.real); 
    vpi_control(vpiFinish); 
  } 

  t.real += 1000.0; 
  next_cb.obj = top; 
  next_cb.time = &t; 
  vpi_register_cb(&next_cb); 
  return 0; 
} 
 
void boot() 
{ 
  struct sigaction newaction; 
  s_cb_data sos_cb = {cbStartOfSimulation, watcher, 0, 0, 0, 0 }; 
 
  newaction.sa_handler = dosig2; 
  sigemptyset (&newaction.sa_mask); 
  newaction.sa_flags = 0; 
  sigaction (SIGUSR2, &newaction, 0); 
  vpi_register_cb(&sos_cb); 
}
