package file_handler;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

public class FileTransfer {
    private ServerSocket fileSocket;
    private byte[] file_data;
    private int file_size;
    private String file_name;

    public FileTransfer(int port, int file_size, String file_name) {
        try {
            fileSocket = new ServerSocket(port);
            this.file_name = file_name;
            this.file_size = file_size;
        } catch (IOException ex) {
            System.err.println("Cannot Connect!");
        }
    }

    public void AcceptFileTransfer() throws IOException{
        fileSocket.accept();
        System.out.println("File transfer connection established");
    }

    public void sendChunks(String filePath, int chunk_size, Socket socket) {
        try {
            DataInputStream in = new DataInputStream(new FileInputStream(filePath));
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());

            byte[] chunk = new byte[chunk_size];
            while((in.read(chunk)) != -1)
            {
                System.out.println(Arrays.toString(chunk));
                dos.write(chunk);
                chunk = new byte[chunk_size];
            }
            in.close();
            dos.close();

        } catch (IOException e) { e.printStackTrace(); }
    }


    public void ReceiveChunk(String filePath, int chunk_size, Socket socket)
    {
        try {
            //file_data = new byte[100];
            DataInputStream in = new DataInputStream(socket.getInputStream());
            FileOutputStream fos = new FileOutputStream(filePath, true);

            byte[] chunk = new byte[chunk_size];
            int i = 0;
            while (in.read(chunk) != -1)
            {
                System.out.println(Arrays.toString(chunk));
                fos.write(chunk);
            }

            in.close();
        } catch (IOException e) { e.printStackTrace(); }
    }
}
