//
// Copyright (c) 2016-2018, Karbo developers
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson/jansson.h>

#include "MinerAPI.h"

const unsigned int Miner::rpc_id = 123;
const char Miner::rpc_v[] = "2.0";

Miner::Miner(char host[HOSTLEN], unsigned int port, unsigned int type){
  this->api_status = false;
  this->res_status = false;
  memset(this->host, 0, HOSTLEN);
  strncpy (this->host, host, sizeof(this->host));
  this->port = port;
  this->type = type;
  this->doErase();
  //this->client_rpc("http://192.168.2.50:3333", "{\"id\": 0,\"jsonrpc\":\"2.0\",\"method\":\"miner_getstat1\"}");
}

Miner::~Miner(){
}

void Miner::doErase(){
  this->farm.hashrate = 0;
  this->farm.rejects = 0;
  this->farm.shares = 0;
  this->farm.times = 0;
  for (unsigned int n = 0; n < GPUS; n++){
    this->farm.units[n].fan = 0;
	this->farm.units[n].hashrate = 0;
	this->farm.units[n].temp = 0;
  }
}

void Miner::explode(const char str[64]){
  const char sub = 0x3B; // ASCII code ';'
  unsigned int i = 0;
  unsigned int n = 0;
  unsigned int k = 0;
  memset(this->explode_buff, 0, 64);
  while (str[n] != 0 && n < 64){
    if (str[n] != sub){
      this->explode_buff[k][i] = str[n];
      i++;
	} else {
      i = 0;
      k++;
	}
    n++;
  }
}

size_t Miner::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
  size_t realsize = size *nmemb;
  Miner::MemoryStruct *mem = (struct MemoryStruct *) userp;
  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL){
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

char *Miner::client_http(char *url, char *data){
  CURL *curl;
  CURLcode res;
  curl_slist *list = NULL;
  Miner::MemoryStruct chunk;
  chunk.memory = (char*) malloc(1);
  chunk.size = 0;
  list = curl_slist_append(list, "Content-Type: application/json");
  this->api_status = false;
  curl = curl_easy_init();
  if(curl){
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(data));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Miner::WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    curl_slist_free_all(list);
    if(res == CURLE_OK){
      if (chunk.size > 0) this->api_status = true;
	  } else {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
    //std::cout << chunk.memory << std::endl;
    //std::cout << chunk.size << std::endl;
    curl_easy_cleanup(curl);
  }
  return chunk.memory;
}

char *Miner::client_rpc(char *url, char *data){
  CURL *curl;
  CURLcode res;
  curl_slist *list = NULL;
  Miner::MemoryStruct chunk;
  chunk.memory = (char*) malloc(1);
  chunk.size = 0;
  list = curl_slist_append(list, "Host:");
  list = curl_slist_append(list, "Accept:");
  this->api_status = false;
  curl = curl_easy_init();
  if(curl){
    curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Miner::WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    curl_slist_free_all(list);
    if(res == CURLE_OK){
      if (chunk.size > 0) this->api_status = true;
	  } else {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
    std::cout << chunk.memory << std::endl;
    //std::cout << chunk.size << std::endl;
    curl_easy_cleanup(curl);
  }
  return chunk.memory;
}

void Miner::doLoad(){
  if (this->type == CLAYMORE) this->doLoadClaymore(); 
}

void Miner::doLoadClaymore(){
  json_t * json;
  json_t * json_req;
  json_error_t error;
  json_t * tmg_status_obj;
  json_t *rpc_id_obj;
  json_t *rpc_v_obj;
  json_t *result_obj;
  json_t *method_obj;
  json_t *time_obj;
  json_t *buff_obj;
  int buff_n = 0;
  char buff_str[64];
  char port_str[8];
  json_req = json_object();
  rpc_id_obj = json_integer((json_int_t) Miner::rpc_id);
  rpc_v_obj = json_string(Miner::rpc_v);
  method_obj = json_string((char*) "miner_getstat1");
  json_object_set(json_req, "id", rpc_id_obj);
  json_object_clear(rpc_id_obj);
  json_object_set(json_req, "jsonrpc", rpc_v_obj);
  json_object_clear(rpc_v_obj);
  json_object_set(json_req, "method", method_obj);
  json_object_clear(method_obj);
  this->res_status = false;
  memset(buff_str, 0, 64);
  memset(port_str, 0, 8);
  sprintf (port_str, ":%d", this->port);
  strcat(buff_str, "http://");
  strcat(buff_str, this->host);
  strcat(buff_str, port_str);
  json = json_loads(this->client_rpc(buff_str, json_dumps(json_req, 0)), 0, &error);
  json_object_clear(json_req);
  if (this->api_status){
    if(json){
      if (json_is_object(json)){
        tmg_status_obj = json_object_get(json, "error");
        if (json_is_null(tmg_status_obj)){
          rpc_id_obj = json_object_get(json, "id");
          if (json_is_number(rpc_id_obj)){
            if (Miner::rpc_id == (unsigned int) json_number_value(rpc_id_obj)){
              result_obj = json_object_get(json, "result");
              if (json_is_array(result_obj)){
                time_obj = json_array_get(result_obj, 1);
                if (json_is_string(time_obj)){
                  sscanf(json_string_value(time_obj), "%d", &buff_n);
				  farm.times = buff_n;
                  buff_n = 0;
                  json_object_clear(time_obj);
				}
                buff_obj = json_array_get(result_obj, 2);
                if (json_is_string(buff_obj)){
                  memset(buff_str, 0, 64);
                  strncpy (buff_str, json_string_value(buff_obj), sizeof(buff_str));
                  json_object_clear(buff_obj);
                  this->explode(buff_str);
                  sscanf(this->explode_buff[0], "%d", &buff_n);
				  this->farm.hashrate = buff_n;
                  buff_n = 0;
                  sscanf(this->explode_buff[1], "%d", &buff_n);
				  this->farm.shares = buff_n;
                  buff_n = 0;
                  sscanf(this->explode_buff[2], "%d", &buff_n);
				  this->farm.rejects = buff_n;
                  buff_n = 0;
				}
                buff_obj = json_array_get(result_obj, 3);
                if (json_is_string(buff_obj)){
                  memset(buff_str, 0, 64);
                  strncpy (buff_str, json_string_value(buff_obj), sizeof(buff_str));
                  json_object_clear(buff_obj);
                  this->explode(buff_str);
                  for (unsigned int n = 0; n < GPUS; n++){
                    if (strcmp(this->explode_buff[n], "") == 0){ break; }
                    sscanf(this->explode_buff[n], "%d", &buff_n);
					this->farm.units[n].hashrate = buff_n;
                    buff_n = 0;
				  }
				}
                buff_obj = json_array_get(result_obj, 6);
                if (json_is_string(buff_obj)){
                  memset(buff_str, 0, 64);
                  strncpy (buff_str, json_string_value(buff_obj), sizeof(buff_str));
                  json_object_clear(buff_obj);
                  this->explode(buff_str);
                  for (unsigned int n = 0; n < GPUS; n++){
                    if (strcmp(this->explode_buff[n], "") == 0){ break; }
                    sscanf(this->explode_buff[n * 2], "%d", &buff_n);
					this->farm.units[n].temp = buff_n;
                    buff_n = 0;
                    sscanf(this->explode_buff[n * 2 + 1], "%d", &buff_n);
					this->farm.units[n].fan = buff_n;
                    buff_n = 0;
				  }
				}
			  }
			}
		  }
		}
	  }
	}
  }
}