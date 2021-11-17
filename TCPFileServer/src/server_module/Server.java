package server_module;
import file_handler.FileTransfer;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Scanner;

public class Server {
    private static ServerSocket serverSocket;
    private ArrayList<Socket> list;
    private int id;
    private FileTransfer fileTransfer;
    private static final int MAX_BUFFER = 2;



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(5017);
        list = new ArrayList<>();
        fileTransfer = new FileTransfer();
        id = 1;
    }


    //---------------------------Starts Listening for Connections-------------------------------//
    public void startListening() throws IOException {
        while(true)
        {
            Socket connectionSocket = serverSocket.accept();
            list.add(connectionSocket);

            Thread t = new Thread(() -> sendFile(connectionSocket));

            t.start();
            System.out.println("\n\nClient [" + id++ + "] is now connected. No. of worker threads = ");
            System.out.println("IP address of client: " + connectionSocket.getRemoteSocketAddress().toString());
        }
    }

    public void sendFile(Socket socket)
    {
        {
            Scanner sc = new Scanner(System.in);
            fileTransfer.sendChunks(sc.nextLine(), MAX_BUFFER, socket);
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