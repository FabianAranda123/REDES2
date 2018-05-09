package controlador;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.file.Paths;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.Part;
import utils.Util;

/**
 *
 * @author isaac_stark
 */
@javax.servlet.annotation.MultipartConfig
public class UploadImage extends HttpServlet {
    Client c = null;
    
    protected void processRequest(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        Part image = request.getPart("demo-file");
        response.setContentType("text/html;charset=UTF-8");
        System.out.println("controlador.UploadImage.processRequest() " + image.getSize());
        String imgName, imgBase64 = "data:image/jpeg;base64,";
        byte imgContent[];
        
        imgName = Paths.get( image.getSubmittedFileName() ).getFileName().toString();
        try( InputStream is = image.getInputStream() ) {
            imgContent = new byte[is.available()];
            is.read( imgContent, 0, is.available() );
            imgBase64 += Base64.getEncoder().encodeToString( imgContent );
        }

        try( PrintWriter out = response.getWriter() ) {
            /* TODO output your page here. You may use following sample code. */
            out.println("<!DOCTYPE html>");
            out.println("<html>");
            out.println("<head>");
            out.println("<title>Servlet UploadImage</title>");
            out.println("</head>");
            out.println("<body bgcolor='#848484'>");
            out.println("<h1>Servlet UploadImage at " + request.getContextPath() + "</h1>");
            out.println("<h1>Servlet UploadImage at " + imgName + " " + image.getSize() + "</h1>");
            out.println("<h1>Servlet UploadImage at " + Util.isWorkerConnect( "127.0.0.1" ) + "</h1>");
            
            out.println("<h1 id='123' aling='center'> Esta pagina se redigira en 5 segundos </h1>");
            out.println("<img src='" + imgBase64 + "'><img>");
            out.println("<script src=\"assets/js/jquery.min.js\"></script>\n" +
"        <script src=\"assets/js/jquery.scrollex.min.js\"></script>\n" +
"        <script src=\"assets/js/jquery.scrolly.min.js\"></script>");
            out.println("<script src=\"assets/js/redireccion.js\"></script>");
            
            out.println("</body>");
            out.println("</html>");
        }
        
        ConnectionMaster cm = new ConnectionMaster( new Client( "127.0.1.1", 5000 ) );
        cm.sendFile( image );
        //connectWithWorker( imgName, "localhost", 8080 );        
    }

    
    
    /* Ejemplo extraido de http://www.theserverside.com/news/thread.tss?thread_id=21884
    ** http://programacionextrema.com/2015/11/26/realizar-una-peticion-post-en-java/*/
    private void connectWithWorker( String image, String IP, int port ) {
        try {
            URL url = new URL("http://" + IP + ":" + Integer.toString( port ) + "/ProyectoRedes2Worker/Worker");
            Map<String, Object> params = new LinkedHashMap<>();
            
            params.put("image", image );
            StringBuilder postData = new StringBuilder();
            
            for( Map.Entry<String, Object> param : params.entrySet() ) {
                if( postData.length() != 0 )
                    postData.append("&");
                postData.append( URLEncoder.encode( param.getKey(), "UTF-8" ) );
                postData.append("=");
                postData.append( URLEncoder.encode( String.valueOf( param.getValue() ), "UTF-8" ) );
            }
            
            byte []postDataBytes = postData.toString().getBytes("UTF-8");
            
            HttpURLConnection conn = ( HttpURLConnection ) url.openConnection();
            conn.setRequestMethod("POST");
            conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            conn.setRequestProperty("Content-Length", String.valueOf( postDataBytes.length ) );
            conn.setDoOutput(true);
            conn.getOutputStream().write( postDataBytes );
            
            InputStream stream = conn.getInputStream();
            BufferedInputStream bin = new BufferedInputStream( stream );
            int i;
            
            while( ( i = bin.read() ) != -1 ){
                System.out.write( i );
            }
            
        } catch( MalformedURLException ex ) {
            Logger.getLogger( UploadImage.class.getName() ).log(Level.SEVERE, null, ex);
        } catch( IOException ex ) {
            Logger.getLogger( UploadImage.class.getName() ).log(Level.SEVERE, null, ex);
        }
        
    }
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        processRequest(request, response);
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        processRequest(request, response);
    }

    @Override
    public String getServletInfo() {
        return "Short description";
    }// </editor-fold>

}
