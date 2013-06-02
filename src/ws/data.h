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
#ifndef WSDATA_H_INCLUDED
#define WSDATA_H_INCLUDED

#include "data/vector.h"
#include "nativehttp.h"

namespace ws
{
    struct ws_ent
    {
        const char* uri;
        const char* protocol;
        nativehttp::websock::cb_onConnect on_connect;
        nativehttp::websock::cb_onTxtFrame on_txt_frame;
        nativehttp::websock::cb_onBinFrame on_bin_frame;
    };

    extern bool enabled;
    extern data::vector<ws_ent>* units;

}

#endif // DATA_H_INCLUDED
