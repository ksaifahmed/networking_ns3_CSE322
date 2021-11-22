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
            if(input.equals("-help")) displayCommands();
            else if (input.equals("pul")){
                homeModule.sendServerRequest("uList:");
            }else if(input.equals("ls")){
                homeModule.sendServerRequest("mFiles:");
            }else if(input.contains("ls -o") && input.split(" ").length == 3) {
                try {
                    int ID = Integer.parseInt(input.split(" ")[2]);
                    if(ID >= 1705100 && ID <= 1705120) {
                        homeModule.sendServerRequest("Files:"+input.split(" ")[2]);
                    } else System.out.println("Invalid user id!\n\n");
                } catch (Exception e) {
                    System.out.println("Invalid user id!\n\n");
                }
            }else System.out.println("Invalid command!\n\n");

        }
    }

    private void displayCommands() {
        System.out.println("Commands Supported:");
        System.out.println("Print user list: pul");
        System.out.println("Display messages: dm");
        System.out.println("View my files: ls");
        System.out.println("View other's files: ls -0 user_id");
        System.out.println("\n\n");
    }
}
