
#include <iostream>

#include <jansson/jansson.h>

#include "MinerAPI.h"

int main(){
  json_t *json;
  json_error_t error;
  json = json_load_file("config.json", 0, &error);
  if (json_is_object(json)){
    Miner miner = Miner((char*) json_string_value(json_object_get(json, "host")), (int) json_number_value(json_object_get(json, "port")), CLAYMORE);
    miner.doLoad();
    std::cout << "MinerInfo" << std::endl;
    std::cout << "Uptime: " << miner.farm.times << " min" << std::endl;
    std::cout << "Total Shares: " << miner.farm.shares << std::endl;
    std::cout << "Total Speed: " << miner.farm.hashrate << " H/s" << std::endl;
    std::cout << "Share rejected: " << miner.farm.rejects << std::endl;
    for (unsigned int n = 0; n < GPUS; n++){
      if (miner.farm.units[n].hashrate == 0) break;
      std::cout << "GPU: " << n << std::endl;
      std::cout << "Speed: " << miner.farm.units[n].hashrate << " H/s" << std::endl;
	  std::cout << "Temp: " << miner.farm.units[n].temp << " C" << std::endl;
	  std::cout << "Fan speed: " << miner.farm.units[n].fan << std::endl;
    }
  }
  return 0;
}