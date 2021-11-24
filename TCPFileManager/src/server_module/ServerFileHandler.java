package server_module;

import java.io.DataInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

public class ServerFileHandler {
    private ServerSocket fileSocket;
    private Socket clientSocket;
    private int file_size, chunk_size;
    private String file_name, access, userID;

    public ServerFileHandler(int port, int file_size, int chunk_size, String file_name, String access, String userID) {
        try {
            fileSocket = new ServerSocket(port);
            this.file_name = file_name;
            this.file_size = file_size;
            this.chunk_size = chunk_size;
            this.access = access;
            this.userID = userID;
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

    public void receiveFile() {
        int n_chunks = (file_size /chunk_size);
        int last_chunk = file_size - chunk_size * n_chunks;
        n_chunks++;
//        if(chunk_size > file_size) {
//
//        }else
        try {

            DataInputStream in = new DataInputStream(clientSocket.getInputStream());
            FileOutputStream fos = new FileOutputStream(userID+"/"+access+"/"+file_name, true);
            byte[] chunk = new byte[chunk_size];
            while((in.read(chunk)) != -1)
            {
                System.out.println(Arrays.toString(chunk));
                System.out.println("chunk_size: "+chunk.length);
                fos.write(chunk);
                n_chunks--;
                if(n_chunks == 1) chunk = new byte[last_chunk];
                else chunk = new byte[chunk_size];
            }

//            while (n_chunks-- > 0) {
//                if(n_chunks == 0) {
//                    System.out.println("last: "+last_chunk);
//                    chunk = new byte[last_chunk];
//
//                }else {
//                    System.out.println("chunk: "+chunk_size);
//                    chunk = new byte[chunk_size];
//                }
//                while (in.read(chunk) != -1)
//                {
//                    System.out.println(Arrays.toString(chunk));
//                    System.out.println("chunk_size: "+chunk.length);
//                    fos.write(chunk);
//                }
//            }
            in.close(); fos.close();
        } catch (IOException ex) {
            System.err.println("Upload aborted");
        }
    }
}
