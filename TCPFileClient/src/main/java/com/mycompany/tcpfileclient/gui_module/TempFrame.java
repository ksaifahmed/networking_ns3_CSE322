package com.mycompany.tcpfileclient.gui_module;

import java.awt.EventQueue;
import javax.swing.JFrame;
import java.awt.BorderLayout;
import javax.swing.JPanel;
import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JScrollPane;
import javax.swing.JLabel;
import java.awt.Color;
import javax.swing.JList;

public class TempFrame {

    private JFrame frame;
    /**
     * Create the application.
     */
    public TempFrame() {
        initialize();
    }

    /**
     * Initialize the contents of the frame.
     */
    private void initialize() {
        frame = new JFrame();
        frame.setBounds(100, 100, 835, 458);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.getContentPane().setLayout(new BorderLayout(0, 0));

        JPanel panel = new JPanel();
        frame.getContentPane().add(panel, BorderLayout.NORTH);

        JButton user_list_btn = new JButton("User List");
        user_list_btn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
            }
        });
        panel.add(user_list_btn);

        JButton view_my_files_btn = new JButton("View My Files");
        panel.add(view_my_files_btn);

        JButton view_other_files_btn = new JButton("View Others' Files");
        view_other_files_btn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
            }
        });
        panel.add(view_other_files_btn);

        JButton messages_btn = new JButton("Messages");
        panel.add(messages_btn);

        JScrollPane scrollPane_1 = new JScrollPane();
        frame.getContentPane().add(scrollPane_1, BorderLayout.EAST);

        JLabel message_list_label = new JLabel("My Messages");
        String s[] = new String[2];
        s[0] = "hello";
        s[1] = "world";

        scrollPane_1.setColumnHeaderView(message_list_label);

        JList message_list = new JList(s);
        message_list.setBackground(Color.LIGHT_GRAY);
        scrollPane_1.setColumnHeaderView(message_list);

        JScrollPane scrollPane = new JScrollPane();
        frame.getContentPane().add(scrollPane, BorderLayout.WEST);

        JList user_list = new JList();
        user_list.setBackground(Color.LIGHT_GRAY);
        scrollPane.setViewportView(user_list);

        JLabel user_list_label = new JLabel("User List");
        scrollPane.setColumnHeaderView(user_list_label);


    }

}

