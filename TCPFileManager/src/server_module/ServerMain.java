/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package server_module;

import java.io.IOException;

/**
 *
 * @author USER
 */
public class ServerMain {
    public static void main(String[] args) throws IOException {
        Server server = new Server();
        server.startListening();
    }
}
