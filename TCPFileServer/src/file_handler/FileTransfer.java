package file_handler;

import java.io.*;
import java.net.Socket;
import java.util.Arrays;

public class FileTransfer {
    private byte[] data;

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
            data = new byte[100];
            DataInputStream in = new DataInputStream(socket.getInputStream());
            FileOutputStream fos = new FileOutputStream("client.txt", true);

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
