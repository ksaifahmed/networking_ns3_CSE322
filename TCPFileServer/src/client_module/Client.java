package client_module;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;


public class Client {

    private static Socket socket;
    private static BufferedReader inFromServer;
    private static PrintWriter outToServer;
    private static BufferedReader inputFromUser;

    public Client() throws IOException {
        socket = new Socket("localhost", 5017);
        System.out.println("connected");

        inFromServer = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        outToServer = new PrintWriter(socket.getOutputStream(), true);
        inputFromUser = new BufferedReader(new InputStreamReader(System.in));
    }

    public void initClient()
    {
        Thread readThread = new Thread(() -> read(socket));
        Thread writeThread = new Thread(Client::write);

        writeThread.start();
        readThread.start();
    }


    private static void write()
    {
        System.out.println("Type message:");
        while(true)
        {
            try{
                String sentence = inputFromUser.readLine();
                outToServer.println(sentence);
                outToServer.flush();

            }
            catch(Exception e)
            {
                break;
            }
        }
    }

    private static void read(Socket socket)
    {
        ///System.out.println("Type message:");
        while(true)
        {
            try{
                BufferedReader bf = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                String msgFromServer = bf.readLine();
                System.out.println("Message from Server: "+ msgFromServer);
            }
            catch(Exception e)
            {
                System.out.println("hoynai");
                break;
            }
        }
    }
}

