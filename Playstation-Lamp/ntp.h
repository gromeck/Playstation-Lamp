/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  module to handle the NTP protocol


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

#ifndef __NTP_H__
#define __NTP_H__ 1

#include <Time.h>
#include "config.h"
#include "util.h"

/*
**  ntp sync cycle
*/
#define NTPSYNC_CYCLES 100

/*
**  init the NTP functions
*/
void NtpSetup(void);

/*
**  update the NTP time
*/
void NtpUpdate(void);

/*
**  get the NTTP time
*/
time_t NtpGetTime(void);

/*
   get the frist received timestamp
*/
time_t NtpUpSince(void);

#endif

/**/
