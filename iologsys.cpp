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

#include <stdio.h>
extern "C"{
#include <libusb-1.0/libusb.h>
#include <logsys/control.h>
#include <logsys/usb.h>
#include <logsys/usb.private.h>
#include <logsys/jconf.h>
}

#include "iologsys.h"
#include "debug.h"

IOLogsys::IOLogsys():IOBase(), buf_ptr(0){}

int IOLogsys::Init(struct cable_t *cable, const char *devopt, unsigned int freq){
	bool ready=false;
	if(libusb_init(NULL)!=0){
		fprintf(stderr, "LibUSB error\n");
		return 1;
	}
	logsys_dev=logsys_usb_open(NULL, NULL);
	if(!logsys_dev){
		fprintf(stderr, "Couldn't open USB device!\n");
		return 1;
	}
	logsys_set_vcc(logsys_dev, true);
	logsys_jtag_begin(logsys_dev, MODE_ECHO, &ready);
	if(!ready){
		fprintf(stderr, "Couldn't open JTAG interface!\n");
		return 1;
	}
	return 0;
}

IOLogsys::~IOLogsys(){
	logsys_jtag_end(logsys_dev);
	logsys_usb_close(logsys_dev);
}

void IOLogsys::txrx_block(const unsigned char *tdi, unsigned char *tdo, int length, bool last){
	int base_buf_ptr=buf_ptr, new_buf_ptr;

	while(length!=0&&buf_ptr<sizeof(buf)){
		if(length<8){
			buf[buf_ptr++]=(char)(length-1);
			length=0;
		}else{
			buf[buf_ptr++]=0x07;
			length-=8;
		}
		buf[buf_ptr++]=*tdi;
		tdi++;
	}

	if(last&&length==0)
		buf[buf_ptr-2]|=0x40;

	if(tdo){
		new_buf_ptr=buf_ptr;
		flush();
		for(int x=base_buf_ptr;x<new_buf_ptr;x+=2){
			*tdo=buf[x];
			tdo++;
		}
	}

	if(buf_ptr>sizeof(buf)-1)
		flush();

	if(length>0)
		txrx_block(tdi, tdo, length, last);
}

void IOLogsys::tx_tms(unsigned char *pat, int length, int force){
	while(length!=0&&buf_ptr<sizeof(buf)){
		if(length<8){
			buf[buf_ptr++]=0x80|(char)(length-1);
			length=0;
		}else{
			buf[buf_ptr++]=0x87;
			length-=8;
		}
		buf[buf_ptr++]=*pat;
		pat++;
	}

	if(force || buf_ptr>sizeof(buf)-1)
		flush();

	if(length>0)
		tx_tms(pat, length, force);
}

void IOLogsys::flush(void){
	if(!buf_ptr)
		return;

	int resp=libusb_bulk_transfer(logsys_dev, LOGSYS_OUT_EP1, buf, buf_ptr, NULL, 0);
	if(resp<0){
		fprintf(stderr, "JTAG transmit failed! %s\n", libusb_error_name(resp));
		return;
	}
	resp=libusb_bulk_transfer(logsys_dev, LOGSYS_IN_EP2, buf, buf_ptr, NULL, 0);
	if(resp<0){
		fprintf(stderr, "JTAG receive failed! %s\n", libusb_error_name(resp));
		return;
	}
	buf_ptr=0;
}
