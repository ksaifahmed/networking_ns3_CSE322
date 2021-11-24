package server_module;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

public class ServerDownloadHandler {
    private ServerSocket fileSocket;
    private Socket clientSocket;
    private int file_size, chunk_size;
    private String file_name, down_from_user, access;

    public ServerDownloadHandler(int port, int file_size, int chunk_size, String file_name, String down_from_user, String access) {
        try {
            fileSocket = new ServerSocket(port);
            this.file_name = file_name;
            this.file_size = file_size;
            this.chunk_size = chunk_size;
            this.down_from_user = down_from_user;
            this.access = access;
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

    public void sendFile(Socket commandSocket) {
        System.out.println("sending file...");
        try {
            DataInputStream in = new DataInputStream(new FileInputStream(down_from_user+"/"+access+"/"+file_name));
            DataOutputStream dos = new DataOutputStream(clientSocket.getOutputStream());
            int n_chunks = (file_size /chunk_size);
            int last_chunk = file_size - chunk_size * n_chunks;
            byte[] chunk = new byte[chunk_size];
            n_chunks++;

            try {
                PrintWriter pw = new PrintWriter(commandSocket.getOutputStream());
                pw.println("down_msg?Download Started...");
                pw.flush();
            } catch (IOException e) {
                System.out.println("Could not send file download failure message!");
                e.printStackTrace();
            }

            while((in.read(chunk)) != -1)
            {
                System.out.println(Arrays.toString(chunk));
                System.out.println("chunk_size: "+chunk.length);
                dos.write(chunk);
                n_chunks--;
                if(n_chunks == 1) chunk = new byte[last_chunk];
                else chunk = new byte[chunk_size];
            }
            in.close(); dos.close();
            Thread.sleep(50);
            PrintWriter pw = new PrintWriter(commandSocket.getOutputStream());
            pw.println("down_msg?File Download Successful! Check \"Downloads\" folder");
            pw.flush();
        } catch (Exception ex) {
            System.out.println("Download Aborted");
            try {
                PrintWriter pw = new PrintWriter(commandSocket.getOutputStream());
                pw.println("down_msg?File Download Failed");
                pw.flush();
            } catch (IOException e) {
                System.out.println("Could not send file download failure message!");
                e.printStackTrace();
            }
            ex.printStackTrace();
        }
    }
}
