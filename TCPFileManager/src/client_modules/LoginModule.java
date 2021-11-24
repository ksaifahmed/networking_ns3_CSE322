package client_modules;

import client_main_package.TCPFileManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class LoginModule {
    public LoginModule(Socket socket) {
        //----start listening for server requests------//
        if (socket != null) {
            Thread t = new Thread(() -> {
                try {
                    authListener(socket);
                } catch (IOException ex) {
                    System.err.println("Could not login, system reset!");
                }
            });
            t.start();
        }
    }

    //----sends out username for authentication-----//
    public void sendCred(String username, Socket socket) throws IOException {
        PrintWriter pw = new PrintWriter(socket.getOutputStream());
        System.out.println("sending auth request...");
        username = "auth:" + username;
        pw.println(username);
        pw.flush();
    }

    //----listens to server for auth response-----//
    private void authListener(Socket socket) throws IOException{
        while(true)
        {
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            if(str.equalsIgnoreCase("Yes:")) {   //-----Successful login------//
                System.out.println("Authentication Successful!");
                TCPFileManager.auth = true;
                break;
            }
            else if(str.equalsIgnoreCase("InvalidUser:")) { //------Unregistered User-----//
                System.out.println("Warning: " + str);
            }
            else if(str.equalsIgnoreCase("IPConflict:")) {  //------Multiple user IP conflict-----//
                System.out.println("Warning: " + str);
            }
        }
    }



}
