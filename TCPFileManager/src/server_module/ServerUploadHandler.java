package server_module;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

public class ServerUploadHandler {
    private ServerSocket fileSocket;
    private Socket clientSocket;
    private int file_size, chunk_size;
    private String file_name, access, userID, fileID, requestID;
    private boolean isRequest; //requested upload indicator
    FileOutputStream fos;



    public ServerUploadHandler(int port, int file_size, int chunk_size, String file_name, String access, String userID, String fileID) {
        try {
            fileSocket = new ServerSocket(port);
            this.file_name = file_name;
            this.file_size = file_size;
            this.chunk_size = chunk_size;
            this.access = access;
            this.userID = userID;
            this.fileID = fileID;
            this.isRequest = false;
        } catch (IOException ex) {
            System.err.println("Cannot Connect!");
        }
    }

    public void AcceptFileTransfer() {
        try {
            clientSocket = fileSocket.accept();
            System.out.println("File transfer connection established");
        } catch (IOException e) {
            System.err.println("Cannot establish connection!");
        }
    }

    //sets upload as a request upload!
    public void enableRequest(String rid) {
        this.isRequest = true;
        requestID = rid;
    }


    public void receiveFile() {
        int n_chunks = (file_size /chunk_size);
        int last_chunk = file_size - chunk_size * n_chunks;
        n_chunks++;

        Server.FILE_BUFFER += file_size;
        Server.file_list.put(fileID, userID+"/"+access+"/"+file_name);
        Server.file_ID_list.put(userID+"/"+access+"/"+file_name, fileID);
        try {

            DataInputStream in = new DataInputStream(clientSocket.getInputStream());
            fos = new FileOutputStream(userID+"/"+access+"/"+file_name, true);
            PrintWriter pw = new PrintWriter(clientSocket.getOutputStream());


            byte[] chunk = new byte[chunk_size];
            while((in.read(chunk)) != -1)
            {
//                System.out.println(Arrays.toString(chunk));
//                System.out.println("chunk_size: "+chunk.length);
                fos.write(chunk);

                //System.out.println("buffer: "+Server.FILE_BUFFER);

                //for testing setSoTimeOut
                //if (n_chunks == 3) Thread.sleep(10100);

                pw.println("ack?"); pw.flush();
                n_chunks--;
                if(n_chunks == 0) break;
                if(n_chunks == 1) chunk = new byte[last_chunk];
                else chunk = new byte[chunk_size];
            }
            fos.close();
            File file = new File(userID+"/"+access+"/"+file_name);
            System.out.println("FileSize:"+file_size + ", Actual Received Size: "+file.length());

            //upload success
            if(file_size == file.length()) {
                if(isRequest) {
                    String message = "ReqID: " + requestID + ", Requested file [id = " + fileID + "] uploaded by " + userID +"~";
                    Server.addMessage(message, requestID);
                }
                pw.println("up_done!"); pw.flush();

            }

            else {
                pw.println("up_file_corrupt!"); pw.flush();
                if(file.exists()) {
                    file.delete(); //delete file
                    Server.file_list.remove(fileID);
                    Server.file_ID_list.remove(userID+"/"+access+"/"+file_name);
                    System.out.println("Uploaded File DELETED!!");
                }
            }

            Server.FILE_BUFFER -= file_size;
            pw.close(); in.close();

            //closing this socket
            try {
                Thread.sleep(50);
                fileSocket.close();
            } catch (Exception e) {
                System.out.println("Could not close file transfer socket");
            }


        } catch (Exception ex) {
            System.err.println("Upload connection lost");
            Server.FILE_BUFFER -= file_size;
            Server.file_list.remove(fileID);
            Server.file_ID_list.remove(userID+"/"+access+"/"+file_name);
            try {
                fos.close();
            } catch (Exception fos_e) {
                System.out.println("Could not close fos!");
            }

            File file = new File(userID+"/"+access+"/"+file_name);
            System.out.println("name: "+file.getName());
            if(file.exists()) {
                file.delete(); //delete file
                System.out.println("Uploaded File DELETED!!");
            }
            //ex.printStackTrace();

            //closing this socket
            try {
                Thread.sleep(50);
                fileSocket.close();
            } catch (Exception e) {
                System.out.println("Could not close file transfer socket");
            }
        }
    }
}
