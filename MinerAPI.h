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


#ifndef MINERSAPI_H_INCLUDED
#define MINERSAPI_H_INCLUDED

#define HOSTLEN 32
#define GPUS 12

#define CLAYMORE 0

struct Unit {
  unsigned int hashrate;
  unsigned int fan;
  unsigned int temp;
};

struct Farm {
  unsigned int times;
  unsigned int hashrate;
  unsigned int shares;
  unsigned int rejects;
  Unit units[GPUS]; 
};

class Miner {
  public:
    Farm farm;
    Miner(char host[HOSTLEN], unsigned int port, unsigned int type);
    ~Miner();
    void doLoad();
  private:
    struct MemoryStruct {
      char *memory;
      size_t size;
    };
    bool api_status;
    bool res_status;
    char host[HOSTLEN];
    unsigned int port;
    unsigned int type;
    static const unsigned int rpc_id;
    static const char rpc_v[];
    char explode_buff[8][8];
    void doErase();
    void explode(const char str[64]);
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
    char *client_http(char *url, char *data);
    char *client_rpc(char *url, char *data);
    void doLoadClaymore();
};

#endif