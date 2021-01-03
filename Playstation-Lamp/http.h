/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  module to handle the HTTP stuff


  This file is part of Playstation-Lamp.

  Playstation-Lamp is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Playstation-Lamp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Playstation-Lamp.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef __HTTP_H__
#define __HTTP_H__ 1

#include "config.h"
#include "util.h"

/*
   user for the Basic Auth
*/
#define HTTP_WEB_USER   "admin"

/*
**  setup the HTTP web server:w
*/
void HttpSetup(void);

/*
**	handle incoming HTTP requests
*/
void HttpUpdate(void);

/*
  return the time in seconds since the last HTTP request
 */
int HttpLastRequest(void);

#endif

/**/
