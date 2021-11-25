package client_main_package;

import java.io.*;
import java.net.Socket;
import java.util.Arrays;

public class ClientFileHandler {
    private Socket socket, og_socket;
    private int file_size, chunk_size;
    private String file_name;

    public ClientFileHandler(int port, int file_size, int chunk_size, String file_name, Socket og_socket) {
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
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            byte[] chunk = new byte[chunk_size]; int i = 1;
            socket.setSoTimeout(30000);
            while((in.read(chunk)) != -1)
            {
                //System.out.println(Arrays.toString(chunk));
                //System.out.println("chunk_size: "+chunk.length);
                dos.write(chunk);
                String str = br.readLine();
//                if(str.equals("ack?")) {
//                    if(n_chunks == 1) System.out.println("Acknowledgement: LAST Chunk "+ i++ + " received by server! UPLOAD COMPLETE");
//                    else System.out.println("Acknowledgement: Chunk "+ i++ + " received by server!");
//                }
                n_chunks--;
                if(n_chunks == 1) chunk = new byte[last_chunk];
                else chunk = new byte[chunk_size];
            }
            String str = br.readLine();
            System.out.println(str);
            if(str.equals("up_done!")) System.out.println("Upload successful!");
            else if(str.equals("up_file_corrupt!")) System.out.println("Upload file size does not match!");

            in.close(); dos.close(); br.close(); socket.close();
        } catch (Exception ex) {
            System.out.println("Upload Aborted...File Removed From Server");
            //ex.printStackTrace();
            try {
                socket.close();
            } catch (IOException e) {
                System.out.println("Could not close file transfer connection");
            }
        }
    }



    public void receive() {
        int n_chunks = (file_size /chunk_size);
        int last_chunk = file_size - chunk_size * n_chunks;
        n_chunks++;

        try {
            DataInputStream in = new DataInputStream(socket.getInputStream());
            FileOutputStream fos = new FileOutputStream("Downloads/"+file_name, true);
            byte[] chunk = new byte[chunk_size];
            while((in.read(chunk)) != -1)
            {
//                System.out.println(Arrays.toString(chunk));
//                System.out.println("chunk_size: "+chunk.length);
                fos.write(chunk);
                n_chunks--;
                if(n_chunks == 1) chunk = new byte[last_chunk];
                else chunk = new byte[chunk_size];
            }
            in.close(); fos.close(); socket.close();
        } catch (IOException ex) {
            ex.printStackTrace();
            try {
                socket.close();
            } catch (IOException e) {
                System.out.println("Could not close file transfer connection");
            }
        }
    }
}
