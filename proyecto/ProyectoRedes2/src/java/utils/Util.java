/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package utils;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author isaac_stark
 */
public class Util {
    
    public static boolean isWorkerConnect( String IP ) {
        boolean isConnect = false;
        
        try {
            
            final InetAddress host = InetAddress.getByName( IP );
            isConnect = host.isReachable( 3000 );
            System.out.println("host.isReachable(1000) = " + isConnect );
            
        } catch( UnknownHostException ex ) {
            Logger.getLogger( Util.class.getName()).log(Level.SEVERE, null, ex );
            
        } catch( IOException ex ) {
            Logger.getLogger( Util.class.getName()).log(Level.SEVERE, null, ex );
            
        }
        
        return isConnect;
    }
}
