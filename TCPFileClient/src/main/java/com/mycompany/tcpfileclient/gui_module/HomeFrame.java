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
public class HomeFrame extends JFrame{
    private JLabel inputLabel;
    public HomeFrame(Socket socket)
    {
        //init ui elements
        initUIElements();
        //add ui elements
        addUIElements();
        //set window/frame params
        setWindowParams();
        //init listener
        initListenerThread(socket);
    }

    private void initUIElements() {
        inputLabel = new JLabel("Welcome Home!");
        inputLabel.setVisible(true);
        inputLabel.setForeground(Color.DARK_GRAY);
        inputLabel.setBounds(100, 80, 120, 20);
    }

    private void addUIElements() {
        this.add(inputLabel);
    }

    private void setWindowParams() {
        this.setPreferredSize(new Dimension(400, 300));
        this.setLayout(null);
        this.setTitle("Home");
        this.setResizable(true);
        this.setDefaultCloseOperation(EXIT_ON_CLOSE);
        this.pack();
        this.setExtendedState(JFrame.MAXIMIZED_BOTH);
        this.setVisible(true);
        this.revalidate();
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
