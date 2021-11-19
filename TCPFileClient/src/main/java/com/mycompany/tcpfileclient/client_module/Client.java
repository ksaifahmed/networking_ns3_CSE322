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
    //private static FileTransfer fileTransfer;
    
    public Socket getSocket(){ return socket; }

    public Client() throws IOException {
        socket = new Socket("localhost", 5017);
        System.out.println("connected to server:" + socket.getRemoteSocketAddress().toString());
        
//        Thread t = new Thread(() -> {
//            try {
//                authListener();
//            } catch (IOException ex) {
//                ex.printStackTrace();
//            }
//        });
//        t.start();
        
    }
    
    public void authenticate(String username) throws IOException {
        PrintWriter pw = new PrintWriter(socket.getOutputStream());
        
        
        System.out.println("sending...");
        pw.println(username);
        pw.flush();
        
        
    }
    
    public void authListener() throws IOException{
        while(true)
        {
            System.out.println("hey");
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            System.out.println("wadup");
            System.out.println(str+"ab");
            if(str.equalsIgnoreCase("Yes:")) {
                TCPFileClient.auth = true;
                System.out.println("Logged in!!");
            }else {
                LoginFrame.messageLabel.setVisible(true);
                System.out.println("received: " + str);

            } 
        }
    }
}