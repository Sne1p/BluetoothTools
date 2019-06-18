#include "bluetoothtools.h"

int main()
{
    BluetoothTools bt ;
    std::vector< std::pair < std::string , std::string > > devices ;

    size_t num ;
    std::cout << "Start Scanning..." << std::endl ;

    num = bt.scan_devices ( devices , 25 ) ;

    for ( unsigned int i = 0 ; i < num ; i++ )
    {
        std::cout << i << ", "
                  << "ADDRESS : " << devices [ i ].first
                  << ", "
                  << "NAME : " << devices [ i ].second
                  << std::endl ;
    }

    if ( num == 0 )
    {
        std::cout << " device not found " << std::endl ;
        return -1 ;
    }

    std::cout << "Select device: " << std::endl ;
    unsigned int k ;
    std::cin >> k ;



    int status = bt.pair_device ( devices [ k ].first , "0000" ) ;

    if ( status != 0 )
    {
        std::cout << "pairing error =(" << std::endl ;
        return -1 ;
    }

    std::cout << "Pairing successful" << std::endl ;

    status = bt.connect_device( devices [ k ].first );
    if ( status != 0 )
    {
        std::cout << "connect error =(" << std::endl ;
    }


    while ( true )
    {
        char buff [ 256 ] = { } ;
        bt.recv ( buff, 256 );
        std::cout << buff << std::endl ;
    }

    return 0 ;
}

