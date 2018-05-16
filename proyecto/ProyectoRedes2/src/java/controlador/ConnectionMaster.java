/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package controlador;

import java.io.IOException;
import java.io.*;
import javax.servlet.http.Part;

/**
 *
 * @author isaac_stark
 */
public class ConnectionMaster {
    private Client c;
    private final char COMPLETE = '0';  //1 byte
    private final char SENDING = '1';  //1 byte
    private final char REQUEST = '2';  //1 byte
    private final int TAM_DATA = 4;    //4 byte 
    private final int NAME_FILE = 30;  //30 byte
    private final int TAM_HEADER = TAM_DATA + NAME_FILE + 1;
    private final int DATA = 100;      //100byte
    private final int TAM_TRAMA = 135; //135 byte
    
    public ConnectionMaster( Client c ) {
        this.c = c;
    }
    
    public void getFile( String nameFile ) {
        try {
            OutputStream out = c.getClient().getOutputStream();
            String flag = "GET_FILE_XD";
            //out.write(b);
            flag = completeStringFormat( flag );
            nameFile = completeStringFormat( nameFile );
            
            out.write( flag.getBytes(), 0, flag.getBytes().length );
            out.write( nameFile.getBytes(), 0, nameFile.getBytes().length );
            
            out.flush();
            out.close();
            
        } catch(IOException ioe) {
            System.out.println("Exception during communication. Server probably closed connection.");
            
        } finally {
            try {
                // Close the socket before quitting
                c.getClient().close();
            } catch(IOException e) {
                e.printStackTrace();
            }                
        }
    }
    
    public void sendFile( Part image ) {
        try {
            byte[] bytes = new byte[100];
            BufferedInputStream bis = new BufferedInputStream( image.getInputStream() );
            OutputStream out = c.getClient().getOutputStream();
            int count,file_size;
            byte sen[] = new byte[TAM_TRAMA];
            
            String msg, nameFile = completeStringFormat( image.getName() );
            out.write( nameFile.getBytes(), 0, nameFile.getBytes().length );
            
            while( ( count = bis.read( bytes ) ) > 0 ) {
                System.out.println(count);
                
                //System.out.println("msg " + msg );
                /*System.arraycopy( msg.getBytes(), 0, sen, 0, msg.getBytes().length );
                System.arraycopy( bytes, 0, sen, msg.getBytes().length, bytes.length );
                c.send( sen );*/
                out.write( bytes, 0, count );
            }
            
            out.flush();
            out.close();
            bis.close();
            
        } catch(IOException ioe) {
            System.out.println("Exception during communication. Server probably closed connection.");
            
        } finally {
            try {
                // Close the socket before quitting
                c.getClient().close();
            } catch(IOException e) {
                e.printStackTrace();
            }                
        }
    }
    
    String p = "\0";
    private String completeStringFormat( String name ) {
        if( name.length() < 30 ) {
            int padding = 30 - name.length();
            String concat = name;
            
            for(  int i = 0; i < padding; i++ ) {
                concat += p;
            }
            
            return concat;
            
        } else {
            return name;
        }
    }
    
    private String completeInt( int num ) {
        String snum = Integer.toString( num );
        
        if( snum.length() < 4 ) {
            int k = 4 - snum.length();
            String ssnum = "";
            
            for( int i = 0; i < k; i++ ) {
                ssnum += p;
            }
            
            return ssnum + snum;
        }
        
        return snum;
    }
}
