package com.mycompany.tcpfileclient.gui_module;

import com.mycompany.tcpfileclient.TCPFileClient;
import com.mycompany.tcpfileclient.client_module.Client;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import javax.swing.*;

import static javax.swing.WindowConstants.EXIT_ON_CLOSE;


/**
 *
 * @author USER
 */
public class HomeFrame extends JFrame{
    private JButton user_list_btn, message_btn, my_files_btn, other_files_button;
    private JPanel message_panel, user_list_panel, files_panel, upload_panel, button_panel;

    public HomeFrame(Socket socket)
    {
        //set window/frame params
        setWindowParams();
        //init ui elements
        initUIElements(socket);
        //add ui elements
        addUIElements();
        //init listener
        initListenerThread(socket);

        this.setVisible(true);
        this.revalidate();
    }

    private void initUIElements(Socket socket) {
        user_list_btn = new JButton("User List");
        user_list_btn.setVisible(true);
        user_list_btn.setBounds(100, 80, 130, 25);
        user_list_btn.addActionListener(e -> {
            sendServerRequest("uList:", socket);
        });

        message_btn = new JButton("Messages");
        message_btn.setVisible(true);
        message_btn.setBounds(100, 80, 130, 25);
        message_btn.addActionListener(e -> {
            //sendServerRequest("uList:", socket);
        });

        my_files_btn = new JButton("My Files");
        my_files_btn.setVisible(true);
        my_files_btn.setBounds(100, 80, 130, 25);
        my_files_btn.addActionListener(e -> {
            //sendServerRequest("uList:", socket);
        });

        other_files_button = new JButton("Other Files");
        other_files_button.setVisible(true);
        other_files_button.setBounds(100, 80, 130, 25);
        other_files_button.addActionListener(e -> {
            //sendServerRequest("uList:", socket);
        });

        message_panel = new JPanel();
        user_list_panel = new JPanel();
        files_panel = new JPanel();
        upload_panel = new JPanel();
        button_panel = new JPanel();

        message_panel.setPreferredSize(new Dimension(300, 170));
        user_list_panel.setPreferredSize(new Dimension(300, 170));
        files_panel.setPreferredSize(new Dimension(300, 170));
        upload_panel.setPreferredSize(new Dimension(300, 200));
        button_panel.setPreferredSize(new Dimension(300, 60));

    }

    private void addUIElements() {
        //adding panels and layouts
        this.setLayout(new BorderLayout());
        this.add(message_panel, BorderLayout.WEST);
        this.add(user_list_panel, BorderLayout.EAST);
        this.add(button_panel, BorderLayout.NORTH);
        this.add(files_panel, BorderLayout.CENTER);
        this.add(upload_panel, BorderLayout.SOUTH);

        button_panel.add(user_list_btn);
        button_panel.add(message_btn);
        button_panel.add(my_files_btn);
        button_panel.add(other_files_button);
    }

    private void setWindowParams() {
        //frame params
        this.setPreferredSize(new Dimension(1280, 720));
        this.setLayout(null);
        this.setTitle("Home");
        this.setResizable(true);
        this.setDefaultCloseOperation(EXIT_ON_CLOSE);
        this.pack();
        this.setExtendedState(JFrame.MAXIMIZED_BOTH);
    }


    public void sendServerRequest(String string, Socket socket)
    {
        try {
            PrintWriter pw = new PrintWriter(socket.getOutputStream());
            pw.println(string);
            pw.flush();
        } catch(IOException ex) {
            System.err.println("Something went wrong when sending!");
        }
    }


    private void initListenerThread(Socket socket) {
        Thread t = new Thread(() -> {
            try {
                commandListener(socket);
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        });
        t.start();
    }


    private void commandListener(Socket socket) throws IOException{
        while(true)
        {
            System.out.println("Init Listener Home!");
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            System.out.println("Server Says:"+str+"/");
            if(str.equalsIgnoreCase("UList:")) {
                break;
            }
        }
    }

}
