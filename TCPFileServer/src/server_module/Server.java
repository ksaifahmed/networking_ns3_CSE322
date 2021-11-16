package server_module;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;

public class Server {
    private static ServerSocket serverSocket;
    private ArrayList<Socket> list;
    private int id;



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

            Thread t = new Thread(){
                @Override
                public void run(){
                    initTransmission(connectionSocket);
                }
            };

            t.start();
            System.out.println("Client [" + id++ + "] is now connected. No. of worker threads = ");
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

                System.out.println("Client msg: " +clientRequests);

                clientReplied = clientRequests.toUpperCase();
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