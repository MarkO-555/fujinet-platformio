#ifndef DEVICE_H
#define DEVICE_H

#ifdef BUILD_ATARI
# include "sio/fuji.h"
# include "sio/apetime.h"
# include "sio/cassette.h"
# include "sio/disk.h"
# include "sio/midimaze.h"
# include "sio/modem.h"
# include "sio/network.h"
# include "sio/printer.h"
# include "sio/printerlist.h"
# include "sio/siocpm.h"
# include "sio/voice.h"

# define PRINTER_CLASS sioPrinter

#elif BUILD_CBM
# include "iec/fuji.h"
# include "iec/printer.h"
# include "iec/printerlist.h"

# define PRINTER_CLASS iecPrinter

#elif BUILD_ADAM
# include "adamnet/fuji.h"
# include "adamnet/keyboard.h"
# include "adamnet/printer.h"
# include "adamnet/modem.h"
# include "adamnet/printerlist.h"
# include "adamnet/query_device.h"

# define PRINTER_CLASS adamPrinter

#endif

#endif // DEVICE_H