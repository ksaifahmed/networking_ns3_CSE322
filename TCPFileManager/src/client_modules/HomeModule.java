package client_modules;

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
            }
        }
    }
}
