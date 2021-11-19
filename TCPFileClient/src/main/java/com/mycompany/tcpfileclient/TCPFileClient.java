/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mycompany.tcpfileclient;

import com.mycompany.tcpfileclient.client_module.Client;
import com.mycompany.tcpfileclient.gui_module.LoginFrame;
import java.io.IOException;


/**
 *
 * @author USER
 */
public class TCPFileClient {
    public static Client client;
    public static boolean auth = false;

    public static void main(String[] args) throws IOException { 
        client = new Client();
        LoginFrame frame = new LoginFrame(client.getSocket());

        //waits for successful login
        while(true)
        {
            if(auth) break;
        }

        //logged in successfully
        System.out.println("Logged in!!!");

    }
}
