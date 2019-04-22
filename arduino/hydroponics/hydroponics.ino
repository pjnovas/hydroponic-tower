#include "config.h"

#include "WaterPump.h"
#include "Enviroment.h"
#include "SimpleTimer.h"

WaterPump waterPump;
Enviroment enviroment;
SimpleTimer timer;

void setup() {

  /*
 int timerid1 = timer.setTimeout(5*1000, []() {
    if(waterPump.on(true)) {
      int timerid2 = timer.setTimeout(5*1000, []() {
        waterPump.off();
      });
    }
  });
  */

  timer.setInterval(RATE_ENV_READ, []() {
    enviroment.readData();
  });

  // if enviroment.isDaylight() ? setup waterPump : stop waterPump

  /*
  timer.setInterval(RATE_POST_SERVER, []() {
    waterPump.getData();
    enviroment.getData();
  });
  */
}

void loop() {
  
}
