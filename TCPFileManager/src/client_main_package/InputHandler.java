package client_main_package;

import client_modules.HomeModule;

import java.util.Scanner;

public class InputHandler {
    private final Scanner sc;

    public InputHandler() {
        sc = new Scanner(System.in);
    }

    public void handle(HomeModule homeModule)
    {
        String input;
        while (true)
        {
            input = sc.nextLine();
            if (input.equals("-help")) displayCommands();
            else if (input.equals("pul")){
                homeModule.sendServerRequest("uList:");
            }

        }
    }

    private void displayCommands() {
        System.out.println("Commands Supported:");
        System.out.println("Print user list: pul");
        System.out.println("Display messages: dm");
        System.out.println("View my files: ls");
        System.out.println("View other's files: ls_userID");
        System.out.println("\n\n");
    }
}
