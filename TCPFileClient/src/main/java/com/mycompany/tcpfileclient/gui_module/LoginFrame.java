/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mycompany.tcpfileclient.gui_module;

import com.mycompany.tcpfileclient.TCPFileClient;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;

/**
 *
 * @author USER
 */
public class LoginFrame extends JFrame{
    private static JButton button;
    private static JTextField textField;
    private static JLabel inputLabel;
    public static JLabel messageLabel;
    
    public LoginFrame(Socket socket)
    {
        //----start listening for server requests------//
        if(socket != null) {
            Thread t = new Thread(() -> {
                try {
                    authListener(socket);
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            });
            t.start();
        }
        
        
        textField = new JTextField();
        textField.setSize(200, 30);
        textField.setLocation(100, 100);
                        
        messageLabel = new JLabel("Already logged in using another IP");
        messageLabel.setVisible(false);
        messageLabel.setForeground(Color.red);
        messageLabel.setBounds(100, 180, 200, 20);


        button = new JButton();
        button.setBounds(160, 150, 70, 20);
        button.setText("Login");
        button.addActionListener((ActionEvent e) -> {
            messageLabel.setVisible(false);
            if(textField.getText() != null && socket != null) {
                try {
                    sendCred("auth:"+textField.getText(), socket);
                }catch (IOException ex)
                {
                    System.out.println("Error connecting...");
                }
            }
        });
        
        
        inputLabel = new JLabel("Enter Student ID:");
        inputLabel.setVisible(true);
        inputLabel.setForeground(Color.DARK_GRAY);
        inputLabel.setBounds(100, 80, 120, 20);
        

        this.add(inputLabel);
        this.add(button);
        this.add(textField);
        this.add(messageLabel);
        
        
        this.setPreferredSize(new Dimension(400, 300));
        this.setLayout(null);
        this.setTitle("Please Login");
        this.setResizable(false);
        this.setDefaultCloseOperation(EXIT_ON_CLOSE);

        
        this.pack();
        this.setVisible(true);
        this.revalidate();
        
    }

    //----prevents app launch when server not init----//
    public static void disconnected()
    {
        inputLabel.setText("Server Down!");
        messageLabel.setVisible(false);
        textField.setVisible(false);
        button.setVisible(false);
    }

    //----sends out username for authentication-----//
    private void sendCred(String username, Socket socket) throws IOException {
        PrintWriter pw = new PrintWriter(socket.getOutputStream());
        
        System.out.println("sending...");
        pw.println(username);
        pw.flush();
    }

    //----listens to server for auth response-----//
    private void authListener(Socket socket) throws IOException{
        while(true)
        {
            System.out.println("Init Listener");
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            System.out.println("Response Received");
            System.out.println("Server Says:"+str+"/");
            if(str.equalsIgnoreCase("Yes:")) {   //-----Successful login------//
                LoginFrame.messageLabel.setVisible(false);
                TCPFileClient.auth = true;
                System.out.println("Authentication Successful!");
                break;
            }
            else if(str.equalsIgnoreCase("InvalidUser:")) { //------Unregistered User-----//
                LoginFrame.messageLabel.setText("Invalid User ID");
                LoginFrame.messageLabel.setVisible(true);
                System.out.println("received: " + str);
            }
            else if(str.equalsIgnoreCase("IPConflict:")) {  //------Multiple user IP conflict-----//
                LoginFrame.messageLabel.setText("Already logged from another IP!");
                LoginFrame.messageLabel.setVisible(true);
                System.out.println("received: " + str);
            }
        }
    }
   
}
