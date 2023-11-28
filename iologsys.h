/* JTAG Logsys Download Cable I/O

Copyright (C) 2022 Bence Csókás

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef IOLOGSYS_H
#define IOLOGSYS_H

#include <libusb-1.0/libusb.h>
//#include <logsys/common.h>

#include "iobase.h"
#include "cabledb.h"

#define TX_BUF (128)

class IOLogsys : public IOBase
{
 protected:
  void txrx_block(const unsigned char *tdi, unsigned char *tdo, int length, bool last);
  void tx_tms(unsigned char *pat, int length, int force);

 public:
  IOLogsys();
  ~IOLogsys();
  int  Init(struct cable_t *cable, const char * serial, unsigned int freq);
  void flush(void);

 private:
  libusb_device_handle* logsys_dev;
  unsigned char buf[TX_BUF];
  int buf_ptr;
};


#endif // IOLOGSYS_H
