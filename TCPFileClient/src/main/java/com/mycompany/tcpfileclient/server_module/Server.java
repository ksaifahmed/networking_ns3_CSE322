package com.mycompany.tcpfileclient.server_module;


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;


public class Server {
    private static ServerSocket serverSocket;
    private ArrayList<Socket> list;
    private int id;
    private static final int MAX_BUFFER = 100;
    
    private final String username = "1705110";



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(5017);
        list = new ArrayList<>();
        id = 1;
    }


    //---------------------------Starts Listening for Connections-------------------------------//
    public void startListening() throws IOException {
        while(true)
        {
            Socket connectionSocket = serverSocket.accept();
            list.add(connectionSocket);

            Thread t = new Thread(() -> initTransmission(connectionSocket));

            t.start();
            System.out.println("\n\nClient [" + id++ + "] is now connected. No. of worker threads = ");
            System.out.println("IP address of client: " + connectionSocket.getRemoteSocketAddress().toString());
        }
    }


    //---------------------------Starts Data Transmission -------------------------------//
    public void initTransmission(Socket connectionSocket) {
        while(true)
        {
            String clientRequests;
            String clientReplied;

            try
            {
                BufferedReader inFromClient = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
                PrintWriter outToClient2 = new PrintWriter(connectionSocket.getOutputStream());
                clientRequests = inFromClient.readLine();

                System.out.println("Client msg: " +clientRequests+"ab");
                if(clientRequests.equals(username))
                    clientReplied = "Yes:";
                else clientReplied = "No!";

                outToClient2.println(clientReplied);
                outToClient2.flush();

            }catch(Exception e)
            {
                break;
            }

        }
    }
    //------------------------End of Transmission---------------------------------------------//



}