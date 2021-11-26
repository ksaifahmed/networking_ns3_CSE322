package client_main_package;

import client_modules.HomeModule;

import java.io.File;
import java.net.Socket;
import java.util.Random;
import java.util.Scanner;

public class InputHandler {
    private final Scanner sc;
    private final String username;

    public InputHandler(String username) {
        this.username = username;
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
            }


            else if(input.equals("ls")){
                homeModule.sendServerRequest("mFiles:");
            }


            else if(input.contains("ls -o") && input.split(" ").length == 3) {
                try {
                    int ID = Integer.parseInt(input.split(" ")[2]);
                    if(ID >= 1705100 && ID <= 1705120) {
                        homeModule.sendServerRequest("Files:"+input.split(" ")[2]);
                    } else System.out.println("Invalid user id!\n\n");
                } catch (Exception e) {
                    System.out.println("Invalid user id!\n\n");
                }
            }


            else if(input.contains("up") && input.split(" ").length == 3) {
                String[] keys = input.split(" ");
                if(!keys[0].equals("up")) System.out.println("Invalid command!\n\n");
                else if(keys[2].equals("public") || keys[2].equals("private")) {
                    File file = new File(keys[1]);
                    if(file.isDirectory()) System.out.println("Select a file not a folder!\n\n");
                    if(file.exists()) {
                        homeModule.sendServerRequest("upload?"+file.getName()+"?"+file.length()+"?"+keys[2]+"?"+keys[1]);
                    }else System.out.println("File does not exist!\n\n");
                }else System.out.println("Invalid file access params: either \"public\" or \"private\"\n\n");

            }


            else if(input.contains("down") && input.split(" ").length == 3) {
                String[] keys = input.split(" ");
                if(!keys[0].equals("down")) System.out.println("Invalid command!\n\n");
                else if (keys[1].equals("public") || keys[1].equals("private")) {
                    homeModule.sendServerRequest("down?"+keys[1]+"?"+keys[2]+"?"+username); //down?public?filename?user_id
                }else System.out.println("Invalid file access params: either \"public\" or \"private\"\n\n");
            }


            else if(input.contains("down -o") && input.split(" ").length == 4) {
                String[] keys = input.split(" ");
                if(!keys[0].equals("down") || !keys[1].equals("-o")) System.out.println("Invalid command!\n\n");
                else if(!validID(keys[2])) System.out.println("Invalid user id!\n\n");
                else {
                    homeModule.sendServerRequest("down?public?"+keys[3]+"?"+keys[2]); //down?public?filename?user_id
                }
            }


            else if(input.equals("req")) {
                Scanner temp = new Scanner(System.in);
                System.out.print("Enter a short description: ");
                String desc = temp.nextLine();
                String reqID = getRequestID();
                homeModule.sendServerRequest("req~"+desc+"~"+reqID);
            }


            else if(input.equals("dm")) {
                homeModule.sendServerRequest("display_msg?");
            }


            else System.out.println("Invalid command!\n\n");

        }
    }

    private void displayCommands() {
        System.out.println("Commands Supported:");
        System.out.println("Print user list: pul");
        System.out.println("Display messages: dm");
        System.out.println("View my files: ls");
        System.out.println("View other's files: ls -o user_id");
        System.out.println("Upload file: up file_name access_param");
        System.out.println("Download my file: down access_param file_name");
        System.out.println("Download other's file: down -o user_id file_name");
        System.out.println("\n\n");
    }

    private boolean validID(String id) {
        try {
            int ID = Integer.parseInt(id);
            return ID >= 1705100 && ID <= 1705110;
        }catch (Exception ex) {
            return false;
        }
    }

    private String getRequestID() {
        StringBuilder buffer = new StringBuilder(5);
        Random random = new Random();
        for (int i = 0; i < 7; i++) {
            int ascii = random.nextInt(122-96) + 97;
            buffer.append((char) ascii);
        }

        String rand_str = buffer.toString();
        return username.substring(4)+"_"+rand_str;
    }
}
