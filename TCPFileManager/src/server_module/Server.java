package server_module;


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;


public class Server {
    private static ServerSocket serverSocket;
    private final HashMap<String, Socket> user_list;
    private final HashMap<Socket, String> socket_list;
    private int id;
    private static final int MAX_BUFFER = 100;



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(5017);
        user_list = new HashMap<>();
        socket_list = new HashMap<>();
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
            System.out.println("\n\nClient [" + id++ + "] is now connected.");
            System.out.println("Socket address of client: " + connectionSocket.getRemoteSocketAddress().toString());
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
                    if(clientReply.toString().equals("Yes:")) {
                        user_list.put(clientRequest.split(":")[1], connectionSocket);
                        socket_list.put(connectionSocket, clientRequest.split(":")[1]);
                        File dir1 = new File(clientRequest.split(":")[1]+"/public");
                        File dir2 = new File(clientRequest.split(":")[1]+"/private");
                        if(!dir1.exists()) { //--- if folder doesn't exist, create ---//
                            dir1.mkdirs();
                            dir2.mkdirs();
                        }
                    }
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

                else if(clientRequest.equals("mFiles:")) {
                    //---public files---//
                    File dir = new File(socket_list.get(connectionSocket)+"/public");
                    File[] list = dir.listFiles();
                    clientReply = new StringBuilder("mFiles:?");
                    if(list != null) {
                        for (File f : list) {
                            if (f.isFile()) {
                                System.out.println("File " + f.getName());
                                clientReply.append(f.getName()).append("(public)?");
                            }
                        }
                        if (list.length == 0) clientReply.append("NO_PUBLIC_FILES?");
                    }
                    clientReply.append("----------------------------------?");
                    //---private files---//
                    dir = new File(socket_list.get(connectionSocket)+"/private");
                    list = dir.listFiles();
                    if(list != null) {
                        for (File f : list) {
                            if (f.isFile()) {
                                System.out.println("File " + f.getName());
                                clientReply.append(f.getName()).append("(private)?");
                            }
                        }
                        if (list.length == 0) clientReply.append("NO_PRIVATE_FILES?");
                    }

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