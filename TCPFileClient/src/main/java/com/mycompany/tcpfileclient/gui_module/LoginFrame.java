/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mycompany.tcpfileclient.gui_module;

import com.mycompany.tcpfileclient.TCPFileClient;
import com.mycompany.tcpfileclient.client_module.Client;
import java.awt.Color;
import java.awt.Dimension;
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
import static javax.swing.WindowConstants.EXIT_ON_CLOSE;


/**
 *
 * @author USER
 */
public class LoginFrame extends JFrame{
    private JButton button;
    private JTextField textField;
    private final JLabel inputLabel;
    public static JLabel messageLabel;
    //private Socket socket;
    
    
    public LoginFrame(Socket socket)
    {
        
        Thread t = new Thread(() -> {
            try {
                authListener(socket);
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        });
        t.start();
        
        
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
            //System.out.println(textField.getText());
            //messageLabel.setVisible(true);
            if(textField.getText() != null) {
                //TCPFileClient.username = textField.getText();
                try {
                    sendCred(textField.getText(), socket);
                }catch (IOException ex)
                {
                    System.out.println("Error connecting...");
                }
            }
            else TCPFileClient.username = null;
            
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

    private void sendCred(String username, Socket socket) throws IOException {
        PrintWriter pw = new PrintWriter(socket.getOutputStream());
        
        System.out.println("sending...");
        pw.println(username);
        pw.flush();
    }
    
    private void authListener(Socket socket) throws IOException{
        while(true)
        {
            System.out.println("hey");
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String str = br.readLine();
            System.out.println("wadup");
            System.out.println(str+"ab");
            if(str.equalsIgnoreCase("Yes:")) {
                TCPFileClient.auth = true;
                System.out.println("Logged in!!");
                break;
            }else {
                LoginFrame.messageLabel.setVisible(true);
                System.out.println("received: " + str);

            } 
        }
    }
   
}
