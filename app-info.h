/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#ifndef APPINFO_H
#define APPINFO_H

#ifndef APPLICATION_NAME
#define APPLICATION_NAME "xmodem-send"
#endif

#ifndef ORGANIZATION_NAME
#define ORGANIZATION_NAME "Heavydeck"
#endif

#ifndef ORGANIZATION_DOMAIN
#define ORGANIZATION_DOMAIN "heavydeck.net"
#endif

#ifndef VERSION_MAJOR
#define VERSION_MAJOR 0
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_BUILD
#define VERSION_BUILD 0
#endif

//QSettings keys
#define KEY_LAST_BAUDRATE     "Baudrate"
#define KEY_LAST_PORT         "Port"
#define KEY_LAST_DIRECTORY    "LastDirectory"
#define KEY_LAST_PARITY       "Parity"
#define KEY_LAST_STOP_BITS    "StopBits"
#define KEY_LAST_FLOW_CONTROL "FlowControl"
#define KEY_USE_PKCS_PADDING  "UsePaddingPKCS7"

#endif // APPINFO_H
