package client_main_package;

import client_modules.LoginModule;

import java.io.IOException;
import java.util.Scanner;
import java.util.concurrent.TimeUnit;

public class TCPFileManager {
    public static Client client;
    public static boolean auth = false;

    public static void main(String[] args) throws IOException {
        client = new Client();
        LoginModule login = new LoginModule(client.getSocket());

        if(client.getSocket() == null) {
            System.exit(0);
        }

        //waits for successful login
        Scanner sc = new Scanner(System.in);
        String username;
        while (true){
            try {
                System.out.print("\n\nEnter ID: ");
                username = sc.nextLine();
                login.sendCred(username, client.getSocket());
                TimeUnit.MILLISECONDS.sleep(500);
            } catch (Exception ex) {
                System.err.println("Timer delay error");
                break;
            }
            if(auth) break;
        }

        System.out.println("Logged in");

    }
}
