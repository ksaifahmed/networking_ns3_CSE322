package client_main_package;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Arrays;

public class ClientFileHandler {
    private Socket socket;
    private int file_size, chunk_size;
    private String file_name;

    public ClientFileHandler(int port, int file_size, int chunk_size, String file_name) {
        try {
            this.file_name = file_name;
            this.file_size = file_size;
            this.chunk_size = chunk_size;
            this.socket = new Socket("localhost", port);
            System.out.println("File transfer connection established!");
        } catch (IOException ex) {
            System.err.println("Cannot Connect!");
        }
    }

    public void upload() {
        System.out.println("uploading...");
        try {
            int n_chunks = (file_size /chunk_size);
            int last_chunk = file_size - chunk_size * n_chunks;
            n_chunks++;

            DataInputStream in = new DataInputStream(new FileInputStream(file_name));
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());

            byte[] chunk = new byte[chunk_size];
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
        } catch (IOException ex) {
            System.out.println("Upload Aborted");
        }
    }
}
