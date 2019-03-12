#include "bluetoothtools.h"

GMainLoop * agent_loop ;
GDBusConnection * con  ;

BluetoothTools::BluetoothTools()
{
    this->m_socket = socket ( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM ) ;
}

///
void bluez_device_appeared ( GDBusConnection * sig         ,
                             const gchar     * sender_name ,
                             const gchar     * object_path ,
                             const gchar     * interface   ,
                             const gchar     * signal_name ,
                             GVariant        * parameters  ,
                             gpointer        user_data     )
{
    ///
    ( void ) sig         ;
    ( void ) sender_name ;
    ( void ) object_path ;
    ( void ) interface   ;
    ( void ) signal_name ;

    std::vector< std::pair< std::string , std::string > > &list = * reinterpret_cast< std::vector< std::pair< std::string , std::string > > *>(user_data) ;
    std::pair< std::string , std::string > device ;
    ///
    GVariantIter * interfaces     ;
    const char   * object         ;
    const gchar  * interface_name ;
    GVariant     * properties     ;

    g_variant_get ( parameters , "(&oa{sa{sv}})" , &object , & interfaces ) ;

    while ( g_variant_iter_next ( interfaces , "{&s@a{sv}}" , & interface_name , & properties ) )
    {
        if ( g_strstr_len ( g_ascii_strdown ( interface_name , -1 ) , -1 , "device" ) )
        {
            //g_print ( "\n" ) ;
            const gchar * property_name ;
            GVariantIter i ;
            GVariant * prop_val ;

            g_variant_iter_init ( & i , properties ) ;
            ///
            while ( g_variant_iter_next ( & i , "{&sv}" , & property_name , & prop_val ) )
            {
                if ( strcmp ( property_name , "Address" ) == 0 )
                {
                   //g_print ( "%s  " , g_variant_get_string ( prop_val , NULL ) ) ;
                   device.first =  g_variant_get_string ( prop_val , NULL )  ;
                }
                else if ( strcmp ( property_name , "Name" ) == 0 )
                {
                   //g_print ( "%s  " , g_variant_get_string ( prop_val , NULL ) ) ;
                   device.second = g_variant_get_string ( prop_val , NULL )  ;
                }
            }
            list.push_back ( device ) ;
            g_variant_unref ( prop_val ) ;
        }
        g_variant_unref ( properties ) ;
    }
    return ;
}

///
gboolean timeout_callback ( gpointer data )
{
    g_main_loop_quit ( ( GMainLoop * ) data );
    return FALSE ;
}

///
size_t BluetoothTools::scan_devices ( std::vector< std::pair< std::string, std::string > > & list , int scan_time_sec )
{
    ///
    GDBusConnection * con       ;
    ///
    GMainLoop       * loop      ;
    ///
    guint           iface_added ;

    loop = g_main_loop_new ( NULL , FALSE ) ;
    con  = g_bus_get_sync  ( G_BUS_TYPE_SYSTEM , NULL , NULL ) ;

    ///
    iface_added = g_dbus_connection_signal_subscribe ( con                                  ,
                                                       "org.bluez"                          ,
                                                       "org.freedesktop.DBus.ObjectManager" ,
                                                       "InterfacesAdded"                    ,
                                                        NULL                                ,
                                                        NULL                                ,
                                                        G_DBUS_SIGNAL_FLAGS_NONE            ,
                                                        bluez_device_appeared               ,
                                                        &list                               ,
                                                        NULL                               );
    ///
    GVariant * result ;
    GError   * error = NULL ;

    ///
    result = g_dbus_connection_call_sync ( con                    ,
                                           "org.bluez"            ,
                                           "/org/bluez/hci0"      ,
                                           "org.bluez.Adapter1"   ,
                                           "StartDiscovery"       ,
                                           NULL                   ,
                                           NULL                   ,
                                           G_DBUS_CALL_FLAGS_NONE ,
                                           -1                     ,
                                           NULL                   ,
                                           & error              ) ;

    guint interval = guint ( scan_time_sec ) * 1000 ;
    g_timeout_add ( interval , timeout_callback , loop);
    g_main_loop_run ( loop ) ;
    g_main_loop_unref ( loop ) ;
    ///
    result = g_dbus_connection_call_sync ( con                    ,
                                           "org.bluez"            ,
                                           "/org/bluez/hci0"      ,
                                           "org.bluez.Adapter1"   ,
                                           "StopDiscovery"        ,
                                           NULL                   ,
                                           NULL                   ,
                                           G_DBUS_CALL_FLAGS_NONE ,
                                           -1                     ,
                                           NULL                   ,
                                           & error              ) ;

    return list.size ( ) ;
}

///
void bluez_agent_method_call ( GDBusConnection       * conn       ,
                               const gchar           * sender     ,
                               const gchar           * path       ,
                               const gchar           * interface  ,
                               const gchar           * method     ,
                               GVariant              * params     ,
                               GDBusMethodInvocation * invocation ,
                               void                  * userdata   )
{
    g_print ( "Agent method call: %s.%s()\n" , interface , method ) ;
    GVariant * p = g_dbus_method_invocation_get_parameters ( invocation ) ;
    char * pin = reinterpret_cast< char * > ( userdata ) ;
    if ( !strcmp ( method , "RequestPinCode" ) )
    {
        g_dbus_method_invocation_return_value ( invocation , g_variant_new ( "(s)", pin ) ) ;
        g_main_loop_quit ( agent_loop ) ;
    }

}

///
static const GDBusInterfaceVTable agent_method_table = {
    .method_call = bluez_agent_method_call,
};

///
int bluez_register_agent ( GDBusConnection * con , char * pin )
{
    GError * error = NULL ;
    guint id = 0 ;
    GDBusNodeInfo * info = NULL ;

    static const gchar bluez_agent_introspection_xml [] =
        "<node name='/org/bluez/SampleAgent'>"
        "   <interface name='org.bluez.Agent1'>"
        "       <method name='Release'>"
        "       </method>"
        "       <method name='RequestPinCode'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='s' name='pincode' direction='out' />"
        "       </method>"
        "       <method name='DisplayPinCode'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='s' name='pincode' direction='in' />"
        "       </method>"
        "       <method name='RequestPasskey'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='u' name='passkey' direction='out' />"
        "       </method>"
        "       <method name='DisplayPasskey'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='u' name='passkey' direction='in' />"
        "           <arg type='q' name='entered' direction='in' />"
        "       </method>"
        "       <method name='RequestConfirmation'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='u' name='passkey' direction='in' />"
        "       </method>"
        "       <method name='RequestAuthorization'>"
        "           <arg type='o' name='device' direction='in' />"
        "       </method>"
        "       <method name='AuthorizeService'>"
        "           <arg type='o' name='device' direction='in' />"
        "           <arg type='s' name='uuid' direction='in' />"
        "       </method>"
        "       <method name='Cancel'>"
        "       </method>"
        "   </interface>"
        "</node>";

    info = g_dbus_node_info_new_for_xml ( bluez_agent_introspection_xml , & error ) ;
    if ( error )
    {
        g_printerr ( "Unable to create node: %s\n" , error->message ) ;
        g_clear_error ( & error ) ;
        return 0 ;
    }
    id = g_dbus_connection_register_object ( con                        ,
                                             "/org/bluez/AutoPinAgent"  ,
                                             info->interfaces [ 0 ]     ,
                                             & agent_method_table       ,
                                             pin                        ,
                                             NULL                       ,
                                             & error                   );
    g_dbus_node_info_unref ( info ) ;
    return id ;
}

static int bluez_agent_call_method ( const gchar * method , GVariant * param )
{
    GVariant * result ;
    GError * error = NULL ;

    result = g_dbus_connection_call_sync ( con                       ,
                                           "org.bluez"               ,
                                           "/org/bluez"              ,
                                           "org.bluez.AgentManager1" ,
                                           method                    ,
                                           param                     ,
                                           NULL                      ,
                                           G_DBUS_CALL_FLAGS_NONE    ,
                                           -1                        ,
                                           NULL                      ,
                                           &error                   );
    if ( error != NULL )
    {
        g_print ( "Register %s: %s\n" , "/org/bluez/AutoPinAgent" , error->message ) ;
        return 1 ;
    }

    g_variant_unref ( result ) ;
    return 0 ;
}

static int bluez_register_autopair_agent ( const char * cap )
{
    int rc ;

    rc = bluez_agent_call_method ( "RegisterAgent" , g_variant_new ( "(os)" , "/org/bluez/AutoPinAgent" , cap ) ) ;

    if ( rc )
    {
        return 1 ;
    }

    rc = bluez_agent_call_method ( "RequestDefaultAgent" , g_variant_new ( "(o)" , "/org/bluez/AutoPinAgent" ) ) ;
    if ( rc )
    {
        bluez_agent_call_method ( "UnregisterAgent" , g_variant_new ( "(o)" , "/org/bluez/AutoPinAgent" ) ) ;
        return 1 ;
    }

    return 0 ;
}

///
int register_agent ( char * pin )
{
    int id ;
    int rc ;


    con = g_bus_get_sync ( G_BUS_TYPE_SYSTEM , NULL , NULL ) ;

    if ( con == NULL )
    {
        g_print ( "Not able to get connection to system bus\n" ) ;
        return 1 ;
    }

    id = bluez_register_agent ( con , pin ) ;

    if ( id == 0 )
    {
        g_print ( "Agent not register\n" ) ;
        return 1 ;
    }

    //
    rc = bluez_register_autopair_agent ( "DisplayOnly" ) ;

    if ( rc )
    {
        g_print ( "Not able to register default autopair agent\n" ) ;
        return 1 ;

    }
    return 0 ;
}

///
int BluetoothTools::pair_device ( std::string address , char * pin )
{

    DBusConnection * dbus_conn = nullptr ;
    DBusError dbus_error ;

    ///
    register_agent ( pin ) ;

    // transfom address AA:BB:CC:XX:YY to /org/bluez/hci0/dev_AA_BB_CC_XX_YY
    std::string addr_for_pairing = address;
    std::replace ( addr_for_pairing.begin() , addr_for_pairing.end() , ':' , '_' ) ;
    addr_for_pairing.insert ( 0 , "/org/bluez/hci0/dev_" ) ;


    dbus_error_init ( & dbus_error ) ;
    dbus_conn = ::dbus_bus_get ( DBUS_BUS_SYSTEM, & dbus_error ) ;

    DBusMessage * dbus_msg = nullptr ;
    dbus_bool_t status ;

    //std::cout << "Pairing..." << std::endl ;
    //  /org/bluez/hci0/dev_10_00_E8_D6_AA_0A
    dbus_msg = dbus_message_new_method_call ( "org.bluez" ,  addr_for_pairing.c_str() , "org.bluez.Device1" , "Pair" ) ;
    status =  dbus_connection_send ( dbus_conn , dbus_msg , NULL ) ;
    if ( status != TRUE )
    {
        //std::cout << "Error" << std::endl ;
        return 1 ;
    }
    agent_loop = g_main_loop_new ( NULL , FALSE ) ;
    g_main_loop_run ( agent_loop ) ;
    return 0 ;
}

///
int BluetoothTools::connect_device ( std::string address )
{
    struct sockaddr_rc property = { 0 } ;
    int  status ;

    ///
    property.rc_family = AF_BLUETOOTH ;
    property.rc_channel = ( uint8_t ) 1 ;
    str2ba ( address.c_str() , &property.rc_bdaddr );

    ///
    status = connect ( this->m_socket , ( struct sockaddr * ) & property , sizeof ( property ) ) ;

    if( status != 0 )
    {
        //std::cout << "connect error" << std::endl ;
        return status ;
    }
    return 0 ;
}

///
ssize_t BluetoothTools::recv ( char * data, size_t size )
{
    return ::recv ( this->m_socket, data, size, 0 ) ;
}

///
ssize_t BluetoothTools::send ( const char * data, size_t size )
{
    return ::send ( this->m_socket, data, size, 0 ) ;
}
