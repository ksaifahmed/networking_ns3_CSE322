package client_modules;

import client_main_package.ClientFileHandler;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;


public class HomeModule {
    private final Socket socket;
    private final String username;
    public HomeModule(Socket socket, String username) {
        this.socket = socket;
        this.username = username;
        initListenerThread();
    }

    public void sendServerRequest(String string)
    {
        try {
            PrintWriter pw = new PrintWriter(socket.getOutputStream());
            pw.println(string);
            pw.flush();
        } catch(IOException ex) {
            System.err.println("Something went wrong when sending!");
        }
    }

    private void initListenerThread() {
        Thread t = new Thread(() -> {
            try {
                System.out.println("Welcome "+username+", type -help for commands!");
                commandListener();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        });
        t.start();
    }


    private void commandListener() throws IOException{
        while(true)
        {
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            //System.out.println("Server Says:"+str+"/");
            if(str.contains("uList:")) {
                String[] data = str.split(" ");
                data[0] = "List of users:";
                for(String user: data) System.out.println(user);
                System.out.println("\n\n");
            } else if(str.contains("mFiles:")) {
                String[] data = str.split("\\?");
                data[0] = "-----My Files-----";
                for(String filename:data) System.out.println(filename);
                System.out.println("\n\n");
            } else if(str.contains("Files:1705")) {
                String[] data = str.split("\\?");
                data[0] = "-----Public Files of " + data[0].split(":")[1] + "-----";
                for(String filename:data) System.out.println(filename);
                System.out.println("\n\n");
            } else if (str.contains("up_yes?")) {
                System.out.println("From server: " + str + "\n\n");
                String[] keys = str.split("\\?");
                int chunk_size = Integer.parseInt(keys[1]);
                int filesize = Integer.parseInt(keys[4]);
                int port = Integer.parseInt(keys[6]);
                String file_ID = keys[2], filename = keys[3], access = keys[5];
                ClientFileHandler fileHandler = new ClientFileHandler(port, filesize, chunk_size, filename);
                Thread t = new Thread(fileHandler::upload); t.start();

            } else if(str.contains("down_fail?")) {
                System.out.println(str.split("\\?")[1]+"\n\n");
            } else if(str.contains("down_yes?")) {
                System.out.println("From server: " + str + "\n\n");
                String[] keys = str.split("\\?");
                int chunk_size = Integer.parseInt(keys[1]);
                int filesize = Integer.parseInt(keys[3]);
                int port = Integer.parseInt(keys[2]);
                String filename = keys[4];
                //down_yes?chunk_size?port?file_size?filename
                ClientFileHandler fileHandler = new ClientFileHandler(port, filesize, chunk_size, filename);
                Thread t = new Thread(fileHandler::receive); t.start();
            } else if(str.contains("down_msg?")) {
                System.out.println(str.split("\\?")[1]+"\n\n");
            }
            else if(str.contains("WARNING:")) {
                System.out.println(str+"\n\n");
            }
        }
    }
}
