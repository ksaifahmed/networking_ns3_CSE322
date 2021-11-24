package server_module;


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Random;


public class Server {
    private static ServerSocket serverSocket;
    private final HashMap<String, Socket> user_list;
    private final HashMap<Socket, String> socket_list;
    private int id;
    //-----sizes in KiloBytes-----//
    private static final int MAX_BUFFER = 52428800; //50MB
    private static final int MAX_CHUNK_SIZE = 10240; //10KB
    private static final int MIN_CHUNK_SIZE = 1024; //1KB



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(6890);
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

                else if(clientRequest.contains("Files:1705")) {
                    String user = clientRequest.split(":")[1];
                    File dir = new File(user+"/public");
                    clientReply = new StringBuilder("Files:");
                    clientReply.append(user).append("?");
                    File[] list = dir.listFiles();

                    if(list != null) {
                        for (File f : list) {
                            if (f.isFile()) {
                                System.out.println("File " + f.getName());
                                clientReply.append(f.getName()).append("?");
                            }
                        }
                        if (list.length == 0) clientReply.append("THIS_USER_HAS_NO_PUBLIC_FILES?");
                    }
                    clientReply.append("----------------------------------?");
                }


                else if(clientRequest.contains("upload?") && clientRequest.split("\\?").length == 4) {
                    String[] keys = clientRequest.split("\\?");
                    int filesize = Integer.parseInt(keys[2]), port = getRandomPort(), chunk_size = getRandomChunkSize();
                    String access = keys[3], filename = keys[1];
                    if(isBufferOverflow(filesize)) {
                        clientReply = new StringBuilder("WARNING: server buffer overflow, try later!");
                    } else {
                        ServerUploadHandler fileHandler = new ServerUploadHandler(port, filesize, chunk_size, filename, access, socket_list.get(connectionSocket));
                        System.out.println("Waiting");
                        Thread t = new Thread(() -> {
                            fileHandler.AcceptFileTransfer();
                            fileHandler.receiveFile();
                        }); t.start();
                        clientReply = new StringBuilder("up_yes?"+chunk_size+"?"+getFileID(connectionSocket)+"?"+filename+"?"+filesize+"?"+access+"?"+port);
                        System.out.println("Sent sskdjksjdsk");
                    }
                }


                else if (clientRequest.contains("down?") && clientRequest.split("\\?").length == 4) {
                    //down?public?filename?user_id
                    String[] keys = clientRequest.split("\\?");
                    File file = new File(keys[3]+"/"+keys[1]+"/"+keys[2]);
                    if(file.exists() && !file.isDirectory() && file.getName().equals(keys[2])) {
                        int port = getRandomPort(); //down_yes?chunk_size?port?file_size?filename
                        clientReply = new StringBuilder("down_yes?"+MAX_CHUNK_SIZE+"?"+port+"?"+file.length()+"?"+keys[2]);
                        ServerDownloadHandler fileHandler = new ServerDownloadHandler(port, (int) file.length(), MAX_CHUNK_SIZE, keys[2], keys[3], keys[1]);
                        System.out.println("Waiting");
                        Thread t = new Thread(() -> {
                            fileHandler.AcceptFileTransfer();
                            try {
                                Thread.sleep(50);
                                fileHandler.sendFile(connectionSocket);
                            } catch (Exception e) {
                                System.err.println("Something Went Wrong When Starting Download!");
                                System.exit(0);
                            }
                        }); t.start();
                    }else clientReply = new StringBuilder("down_fail?"+"Specified File Does Not Exist in Server!");
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


    private static boolean isBufferOverflow(int filesize) {
        return false;
    }

    private int getRandomChunkSize() {
        return new Random().nextInt((MAX_CHUNK_SIZE - MIN_CHUNK_SIZE) + 1) + MIN_CHUNK_SIZE;
    }

    private int getRandomPort() {
        return new Random().nextInt((20000 - 7000) + 1) + 7000;
    }

    private String getFileID(Socket socket) {
        Random random = new Random();
        StringBuilder buffer = new StringBuilder(7);
        for (int i = 0; i < 7; i++) {
            int ascii = random.nextInt(122-96) + 97;
            buffer.append((char) ascii);
        }
        String rand_str = buffer.toString();
        return socket_list.get(socket)+"_"+rand_str;
    }


}