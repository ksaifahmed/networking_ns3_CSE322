package com.mycompany.tcpfileclient.client_module;
import java.io.IOException;
import java.net.Socket;



public class Client {
    private static Socket socket;
    public Socket getSocket(){ return socket; }

    public Client() {
        try {
            socket = new Socket("localhost", 5017);
            System.out.println("connected to server:" + socket.getRemoteSocketAddress().toString());
        } catch (IOException e) {
            System.err.println("error in connection");
        }
    }

}