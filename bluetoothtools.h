#ifndef BLUETOOTHTOOLS_H
#define BLUETOOTHTOOLS_H

#include <glib.h>
#include <gio/gio.h>
#include <dbus/dbus.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <algorithm>

class BluetoothTools
{
private:
    ///
    int m_socket ;

public:

    ///
    BluetoothTools ( ) ;

    ///
    size_t scan_devices ( std::vector< std::pair< std::string, std::string > > & list ,  int search_timeout ) ;

    ///
    int pair_device ( std::string address , char * pin ) ;

    ///
    int connect_device ( std::string address) ;

    ///
    ssize_t recv ( char * , size_t ) ;

    ///
    ssize_t send ( const char * , size_t ) ;
};

#endif // BLUETOOTHTOOLS_H
