package server_module;


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;


public class Server {
    private static ServerSocket serverSocket;
    private final HashMap<String, Socket> user_list;
    private int id;
    private static final int MAX_BUFFER = 100;



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(5017);
        user_list = new HashMap<>();
        id = 1;
    }


    //---------------------------Starts Listening for Connections-------------------------------//
    public void startListening() throws IOException {
        while(true)
        {
            Socket connectionSocket = serverSocket.accept();
            //list.add(connectionSocket);

            Thread t = new Thread(() -> initTransmission(connectionSocket));

            t.start();
            System.out.println("\n\nClient [" + id++ + "] is now connected. No. of worker threads = ");
            System.out.println("IP address of client: " + connectionSocket.getRemoteSocketAddress().toString());
        }
    }


    //---------------------------Starts Command/Keyword Transmission -------------------------------//
    public void initTransmission(Socket connectionSocket) {
        while(true)
        {
            String clientRequest;
            StringBuilder clientReply;

            try
            {
                BufferedReader inFromClient = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
                PrintWriter outToClient2 = new PrintWriter(connectionSocket.getOutputStream());
                clientRequest = inFromClient.readLine();

                //received request keywords from client
                System.out.println("Client request: " +clientRequest+"/");

                //--------------cases of different requests handled here----------------
                if(clientRequest.contains("auth:")) //-----LOGIN AUTH REQUEST-------//
                {
                    clientReply = new StringBuilder(authenticateUser(clientRequest.split(":")[1], connectionSocket));
                    if(clientReply.toString().equals("Yes:")) user_list.put(clientRequest.split(":")[1], connectionSocket);
                }

                else if(clientRequest.equals("uList:")) { //-----SEND USER LIST REQUEST-------//
                    clientReply = new StringBuilder("uList: ");
                    String status;
                    for(HashMap.Entry<String, Socket> user : user_list.entrySet()) {
                        status = "(offline)";
                        if(checkConnectionStatus(user.getValue())) {
                            System.out.println(user.getKey()+":"+checkConnectionStatus(user.getValue()));
                            status = "(active)";
                        }
                        clientReply.append(user.getKey()).append(status).append(" ");
                    }
                    System.out.println(clientReply);
                    System.out.println(user_list);
                }
                else clientReply = new StringBuilder("Null:");


                //----send server reply keywords-----//
                outToClient2.println(clientReply);
                outToClient2.flush();

            }catch(Exception e)
            {
                break;
            }

        }
    }
    //------------------------End of Transmission---------------------------------------------//


    //-----------------------Get Auth Result as a String-------------------------------------//
    private String authenticateUser(String username, Socket socket)
    {
        int ID;
        try {
            ID = Integer.parseInt(username);
            if (ID >= 1705000 && ID <= 1705120)
            {
                String str = String.valueOf(ID);
                Socket temp = user_list.get(str);
                if(temp == null) return "Yes:";
                if(!checkConnectionStatus(temp)) return "Yes:";
                System.out.println(temp.getRemoteSocketAddress().toString());
                System.out.println(socket.getRemoteSocketAddress().toString());
                if(temp.getRemoteSocketAddress().toString().equals(socket.getRemoteSocketAddress().toString()))
                    return "Yes:";
                else return "IPConflict:";
            } else return "InvalidUser:";
        } catch (Exception e) {
            return "InvalidUser:";
        }
    }

    //------------------Checks whether a Socket Connection is live---------------------//
    private boolean checkConnectionStatus(Socket skt)
    {
        boolean status;
        try {
            skt.getOutputStream().write(101);
            skt.getOutputStream().flush();
            System.out.println("trying to ping ip: "+skt.getRemoteSocketAddress().toString());
            status = true;
        } catch (IOException ex) {
            System.err.println("unable to ping current user's previous ip: "+ skt.getRemoteSocketAddress().toString());
            status = false;
        }
        return status;
    }





}