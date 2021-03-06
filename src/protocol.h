/*
Copyright (c) 2013 Lukasz Magiera

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/
#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include <deque>
#include <map>
#include <fstream>
#include "utils/time.h"
#include "data/vector.h"
#include "nativehttp.h"
#include "pagemap/pagemap.h"
#include <sys/prctl.h>

using namespace std;

#define CLPROT_HTTP 0
#define CLPROT_WEBSOCKETS 1
#define IS_MACHINE_NETWORK_BYTE_ORDERED (!(((union { unsigned x; unsigned char c; }){1}).c))


void deamonize();
size_t getacmem();
size_t getrsmem();
void goroot();

nativehttp::data::pagedata exec(string uri, nativehttp::rdata *rd);

class mimec
{
	private:
		map<string, string> mimes;
	public:
		mimec();
		string get_ctype(string fn);
};



extern page_mapper pmap;
extern string charset;
extern mimec *mime;
extern nativehttp::data::Ccfg *cfg;
extern int postmax;
extern string default_mime;
extern bool deamonized;
extern ofstream logfile;
extern bool showExit;
extern string conf;
extern string temp_dir;

#endif // PROTOCOL_H_INCLUDED
