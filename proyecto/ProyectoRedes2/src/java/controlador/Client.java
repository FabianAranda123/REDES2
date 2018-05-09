package controlador;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Master Chief
 */
public class Client {
    private Socket client;
    
    public Client( Socket client ) {
        this.client = client;
    }
    
    public Client( String IP, int port ) {
        createConnection( IP, port );
    }
    
    public Client( InetAddress IP, int port ) {
        createConnection( IP.getHostAddress(), port );
    }
    
    private void createConnection( String IP, int port ) {
        try {
            setClient( new Socket( IP, port ) );
            
        } catch( IOException ex ) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    public Socket getClient() {
        return this.client;
    }

    public void setClient(Socket client) {
        this.client = client;
    }
    
    public Boolean send( String msg ) {
        try {
            DataOutputStream salida = new DataOutputStream( this.client.getOutputStream() );
            salida.writeBytes( msg );
            //salida.writeUTF( msg );
            //System.out.println("modelo.Client.send() MENSAJE ENVIADO " + msg );
            
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            
            return false;
        }
        
        return true;
    }
    
    public Boolean send( int msg ) {
        try {
            DataOutputStream salida = new DataOutputStream( this.client.getOutputStream() );
            salida.writeInt( msg );
            
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            
            return false;
        }
        
        return true;
    }
    
    public Boolean send( byte msg[] ) {
        try {
            DataOutputStream salida = new DataOutputStream( this.client.getOutputStream() );
            salida.write( msg );
            
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
            
            return false;
        }
        
        return true;
    }
    
    public String recv() {
        String msg = null;
        
        System.out.println("EN RECV");
        try {
            BufferedReader in = new BufferedReader( new InputStreamReader( getClient().getInputStream() ) );
            msg = in.readLine();
            
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        
        }
        System.out.println("Saliendod e recv con " + msg );
        
        return msg;
    }
    
    public static void main( String args[] ) {
        try {
            String IP = InetAddress.getLocalHost().getHostAddress();
            Client c = new Client( IP, 6666 );
            String msg = "Hola XD";
            
            System.out.println("modelo.Client.main() " + IP + "  " + c.getClient().getPort() );
            try (DataOutputStream dos = new DataOutputStream( c.getClient().getOutputStream() )) {
                dos.writeBytes( msg + "200" + '\0' );
                dos.close();
            }
            
        } catch (UnknownHostException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
