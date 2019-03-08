package start;

import javax.swing.*;

public class HelloWorldSwing {
    public static void main(String args[]) {
        // Create and set up window.
        JFrame frame = new JFrame("HelloWorld");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        JLabel label = new JLabel("Hello World");
        frame.add(label);

        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }

    class Dummy {
        // Just to have another thing to pack in the jar
    }
}
