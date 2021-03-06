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
#include "../net.h"
#include "http/stat.h"
#include "ws/ws.h"

namespace http
{

	namespace bsd
	{
		void reciver()
		{

            char* rqbuf = NULL;

			while (1)
			{
				SOCKET mfd = 0;
				fd_set set;
				struct timeval tv;
				int slr;

				tv.tv_sec = 0;
				tv.tv_usec = 0;

				for (int i = 0; i < http::maxConnections; i++)
				{
					if (http::connected[i] != -1 && !http::ulock[i])
					{
						if (mfd < http::connected[i])
						{
							mfd = http::connected[i];
						}
					}
				}

				FD_ZERO(&set);

				for (int i = 0; i < http::maxConnections; i++)
				{
					if (http::connected[i] != -1 && !http::ulock[i])
					{
						FD_SET(http::connected[i], &set);
					}
				}

				slr = select(mfd + 1, &set, NULL, NULL, &tv);

				for (int i = 0; i < http::maxConnections && (slr > 0); i++)
				{
					if (http::connected[i] != -1 &&
					        !http::ulock[i] &&
					        FD_ISSET(http::connected[i], &set))
					{

                        if(!rqbuf)rqbuf = new char[(HTTP_MAX_USER_HEADER_SIZE + 1)];

						int ra = recv(http::connected[i], (char *) rqbuf, HTTP_MAX_USER_HEADER_SIZE, 0);

						if (ra > 0)
						{
                            if(http::client_protocol[i] == CLPROT_HTTP)
                            {
#ifdef NHDBG
                                if(http::log_detailed)nativehttp::server::log("DETAIL@bsdRecv","Recieved data for HTTP module: "+nativehttp::data::superstring::str_from_int(i));
#endif
                                http::request *trq = new http::request();
                                trq->request = rqbuf;
                                rqbuf = NULL; //Do not ever think about freeing this here

                                ((char*)trq->request)[ra] = '\0';
                                http::statdata::onrecv(ra);

                                trq->uid = i;
                                trq->sender = http::connected[i];
                                http::ulock[i] = true;

                                utils::condex_send_begin(http::cdx_exec);
                                if(http::toexec.push(trq))
                                {
                                    nativehttp::server::err("recive.cpp@bsd","Request push error");
                                }
                                utils::condex_send_end(http::cdx_exec);
                            }
                            else
                            {
#ifdef NHDBG
                                if(http::log_detailed)nativehttp::server::log("DETAIL@bsdRecv","Recieved data for WebSocket module: "+nativehttp::data::superstring::str_from_int(i));
#endif
                                http::statdata::onrecv(ra);
                                ws::rcv_push((unsigned char*)rqbuf, ra, i);
                                rqbuf = NULL;
                            }
						}
						else
						{
							http::bsd::disconnect(i);
#ifdef NHDBG
                            if(http::log_detailed)nativehttp::server::log("DETAIL@bsdRecv","User disconnected: "+nativehttp::data::superstring::str_from_int(i));
#endif
						}

					}
				}
				utils::sleep(1);

			}
		}
	}
}
