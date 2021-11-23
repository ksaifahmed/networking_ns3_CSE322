package client_main_package;
import java.io.IOException;
import java.net.Socket;



public class Client {
    private static Socket socket;
    public Socket getSocket(){ return socket; }

    public Client() {
        try {
            socket = new Socket("localhost", 6890);
            System.out.println("connected to server:" + socket.getRemoteSocketAddress().toString());
        } catch (IOException e) {
            System.err.println("Service Not Available! Error Code: 503");
        }
    }

}