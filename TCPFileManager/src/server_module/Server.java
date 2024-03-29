package server_module;


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;


public class Server {
    private static ServerSocket serverSocket;
    private final HashMap<String, Socket> user_list;
    private final HashMap<Socket, String> socket_list;

    public static final HashMap<String, String> file_list = new HashMap<>();
    public static final HashMap<String, String> file_ID_list = new HashMap<>();
    public static final HashMap<String, StringBuilder> message_list = new HashMap<>(); //user, messages
    public static final HashMap<String, String> requests = new HashMap<>(); //requestID, user

    private int id;
    //-----sizes in KiloBytes-----//
    private static final int MAX_BUFFER = 104857600; //100MB
    private static final int MAX_CHUNK_SIZE = 10240; //10KB
    private static final int MIN_CHUNK_SIZE = 1024; //1KB
    public static int FILE_BUFFER;



    //---------------------------Server Constructor-------------------------------//
    public Server() throws IOException {
        serverSocket = new ServerSocket(6890);
        user_list = new HashMap<>();
        socket_list = new HashMap<>();
        FILE_BUFFER = 0;
        id = 1;
        initMessageList();
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
                                clientReply.append(f.getName()).append("(public), fileID:").append(file_ID_list.get(socket_list.get(connectionSocket)+"/public/"+f.getName())).append("?");
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
                                clientReply.append(f.getName()).append("(private), fileID:").append(file_ID_list.get(socket_list.get(connectionSocket)+"/private/"+f.getName())).append("?");
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
                                clientReply.append(f.getName()).append(", fileID:").append(user).append("/public/").append(f.getName()).append("?");
                            }
                        }
                        if (list.length == 0) clientReply.append("THIS_USER_HAS_NO_PUBLIC_FILES?");
                    }
                    clientReply.append("----------------------------------?");
                }


                else if(clientRequest.contains("upload_req?") && clientRequest.split("\\?").length == 6) {
                    String[] keys = clientRequest.split("\\?");
                    String requestID = keys[5];
                    if(requests.get(requestID) == null) {
                        clientReply = new StringBuilder("WARNING: Request ID Does Not Exist!");
                    } else {
                        int port = getRandomPort();
                        int filesize = Integer.parseInt(keys[2]), chunk_size = getRandomChunkSize();
                        String access = keys[3], filename = keys[1];
                        String file_ID = getFileID(connectionSocket);
                        if(isBufferOverflow(filesize)) {
                            clientReply = new StringBuilder("WARNING: server buffer overflow, try later!");
                        } else {
                            ServerUploadHandler fileHandler = new ServerUploadHandler(port, filesize, chunk_size, filename, access, socket_list.get(connectionSocket), file_ID);
                            fileHandler.enableRequest(requestID);
                            System.out.println("Waiting to start...");
                            Thread t = new Thread(() -> {
                                fileHandler.AcceptFileTransfer();
                                fileHandler.receiveFile();
                            }); t.start();
                            clientReply = new StringBuilder("up_yes?"+chunk_size+"?"+file_ID+"?"+keys[4]+"?"+filesize+"?"+access+"?"+port);
                        }
                    }
                }



                else if(clientRequest.contains("upload?") && clientRequest.split("\\?").length == 5) {
                    String[] keys = clientRequest.split("\\?");
                    int filesize = Integer.parseInt(keys[2]), port = getRandomPort(), chunk_size = getRandomChunkSize();
                    String access = keys[3], filename = keys[1], file_ID = getFileID(connectionSocket);
                    if(isBufferOverflow(filesize)) {
                        clientReply = new StringBuilder("WARNING: server buffer overflow, try later!");
                    } else {
                        ServerUploadHandler fileHandler = new ServerUploadHandler(port, filesize, chunk_size, filename, access, socket_list.get(connectionSocket), file_ID);
                        System.out.println("Waiting to start upload..");
                        Thread t = new Thread(() -> {
                            fileHandler.AcceptFileTransfer();
                            fileHandler.receiveFile();
                        }); t.start();
                        clientReply = new StringBuilder("up_yes?"+chunk_size+"?"+file_ID+"?"+keys[4]+"?"+filesize+"?"+access+"?"+port);
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

                //down?fileID
                else if (clientRequest.contains("down?") && clientRequest.split("\\?").length == 2) {
                    String[] keys = clientRequest.split("\\?");
                    String fileID = keys[1];
                    System.out.println("file path: " + file_list.get(fileID));
                    if(file_list.get(fileID) == null) {
                        clientReply = new StringBuilder("WARNING: File with fileID: "+fileID+" does not exist!");
                    }

                    else if(file_list.get(fileID).split("/").length != 3) {
                        System.err.println("File path error!");
                        clientReply = new StringBuilder("WARNING: File with fileID: "+fileID+" does not exist!");
                    }

                    else if(file_list.get(fileID).split("/")[1].contains("private") && !socket_list.get(connectionSocket).equals(file_list.get(fileID).split("/")[0])) {
                        System.err.println(socket_list.get(connectionSocket)+ " is trying to access private file");
                        clientReply = new StringBuilder("WARNING: File with fileID: "+fileID+" does not exist!");
                    }


                    else {
                        File file = new File(file_list.get(fileID));
                        String[] path_list = file_list.get(fileID).split("/");
                        if(file.exists() && !file.isDirectory()) {
                            int port = getRandomPort(); //down_yes?chunk_size?port?file_size?filename
                            clientReply = new StringBuilder("down_yes?"+MAX_CHUNK_SIZE+"?"+port+"?"+file.length()+"?"+file.getName());
                            ServerDownloadHandler fileHandler = new ServerDownloadHandler(port, (int) file.length(), MAX_CHUNK_SIZE, file.getName(), path_list[0], path_list[1]); //port, filesize, chunk, filename, down_from_user, access
                            System.out.println("Waiting to download...");
                            Thread t = new Thread(() -> {
                                fileHandler.AcceptFileTransfer();
                                try {
                                    Thread.sleep(51);
                                    fileHandler.sendFile(connectionSocket);
                                } catch (Exception e) {
                                    System.err.println("Something Went Wrong When Starting Download!");
                                    System.exit(0);
                                }
                            }); t.start();
                        }else clientReply = new StringBuilder("WARNING: File with fileID: "+fileID+" does not exist!");
                    }
                }


                else if (clientRequest.contains("req~") && clientRequest.split("~").length == 3) {
                    String[] keys = clientRequest.split("~");
                    clientReply = new StringBuilder("req_r?Request Received By Server, Request ID: "+ keys[2]);
                    requests.put(keys[2], socket_list.get(connectionSocket));

                    ///for loop here
                    for(HashMap.Entry<String, StringBuilder> message : message_list.entrySet()) {
                        if(message.getKey().equals(socket_list.get(connectionSocket))) {
                            continue;
                        }
                        String str = "Request from " + socket_list.get(connectionSocket) + ", RequestID: " + keys[2] +", Description: "+keys[1]+"~";
                        message.getValue().append(str);
                    }
                }


                else if(clientRequest.equals("display_msg?")) {
                    String message = message_list.get(socket_list.get(connectionSocket)).toString();
                    message_list.get(socket_list.get(connectionSocket)).setLength(0);
                    message_list.get(socket_list.get(connectionSocket)).append("");
                    if(message.equals("")) clientReply = new StringBuilder("msg_list~You have no new messages!");
                    else clientReply = new StringBuilder("msg_list~"+message);
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
        return FILE_BUFFER + filesize >= MAX_BUFFER;
    }

    private int getRandomChunkSize() {
        return new Random().nextInt((MAX_CHUNK_SIZE - MIN_CHUNK_SIZE) + 1) + MIN_CHUNK_SIZE;
    }

    private int getRandomPort() {
        return new Random().nextInt((20000 - 7000) + 1) + 7000;
    }

    private String getFileID(Socket socket) {
        Random random = new Random();
        StringBuilder buffer = new StringBuilder(5);
        for (int i = 0; i < 7; i++) {
            int ascii = random.nextInt(122-96) + 97;
            buffer.append((char) ascii);
        }

        String rand_str = buffer.toString();
        return socket_list.get(socket).substring(4)+"_"+rand_str;
    }

    private void initMessageList() {
        for(int i=0; i<= 120; i++) {
            message_list.put("1705"+i, new StringBuilder(""));
        }
    }

    public static void addMessage(String message, String requestID) {
        String userID = requests.get(requestID);
        message_list.get(userID).append(message);
    }


}