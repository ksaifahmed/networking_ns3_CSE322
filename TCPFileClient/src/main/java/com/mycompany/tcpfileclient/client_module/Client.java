package com.mycompany.tcpfileclient.client_module;
import com.mycompany.tcpfileclient.TCPFileClient;
import com.mycompany.tcpfileclient.gui_module.LoginFrame;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;



public class Client {
    private static Socket socket;
    public Socket getSocket(){ return socket; }

    public Client() throws IOException {
        try {
            socket = new Socket("localhost", 5017);
            System.out.println("connected to server:" + socket.getRemoteSocketAddress().toString());
        } catch (IOException e) {
            System.err.println("error in connection");
        }
    }
}